#include "src/db/compactor.hpp"

#include <chrono>
#include <cstring>
#include <iterator>
#include <optional>

#include "src/base/log/logging.hpp"
#include "src/base/timestamp.hpp"
#include "src/db/key_format.hpp"
#include "src/db/memtable.hpp"
#include "src/db/request.hpp"
#include "src/db/shared_memtable.hpp"
#include "src/db/sstable_manager.hpp"
#include "src/util/bloomfilter.hpp"
#include "src/util/iouring.hpp"

namespace fver {

namespace db {

void CompWorker::AddSStable(const std::shared_ptr<Memtable>& memtable) {
  task_mtx_.lock();
  bg_wait_to_sync_sstable_.push_back(memtable);
  task_mtx_.unlock();
  cond_.notify_one();
}

void Compactor::Init(const DBConfig& db_config) {
  // sstable 的目录
  db_path_ = db_config.db_path;
  memtable_n_ = db_config.memtable_N;
  task_workers_n_ = db_config.compactor_thread_size;
  max_level_n_ = db_config.max_level_size;
  // 初始化最大层数
  CompWorker::max_level_n_ = db_config.max_level_size;
  CompWorker::memtable_N_ = db_config.memtable_N;
  CompWorker::cur_version_ = db_config.version_;
  LOG_INFO("size: {} worker_size: {}", memtable_n_, task_workers_n_);
  current_comp_index_ = 0;
  // 初始化 CompactorWorker
  bg_comp_workers_.resize(task_workers_n_);
  for (auto& iter : bg_comp_workers_) {
    iter = std::make_shared<CompWorker>();
  }
  //
  LOG_INFO("bg_comp_worker_size: {}", bg_comp_workers_.size());
  // 初始化 sstable_manager
  sstable_manager_ = std::make_shared<SSTableManager>();
  //
  sstable_manager_->Init(db_config);
  for (auto& iter : bg_comp_workers_) {
    iter->sstable_manager_ = sstable_manager_;
    iter->path = db_path_;
  }
}

//
void Compactor::Run() {
  for (auto& iter : bg_comp_workers_) {
    iter->Run();
    iter->isRunning_ = true;
  }
}

void Compactor::SetCompWorkerCompactorRef(
    const std::shared_ptr<Compactor>& compactor) {
  // assert(bg_comp_workers_.size() == 1);
  assert(compactor.get() != nullptr);
  for (auto& iter : bg_comp_workers_) {
    iter->SetCompactorRef(compactor);
  }
}

void Compactor::SetSharedMemTableRef(
    const std::shared_ptr<SharedMemtable>& sharedmemtable) {
  assert(sharedmemtable.get() != nullptr);
  for (auto& iter : bg_comp_workers_) {
    iter->shared_memtable_ref_ = sharedmemtable;
  }
}

void Compactor::SetSharedMemTableViewSSRef(
    const std::shared_ptr<MemTableViewManager>& manager) {
  assert(manager.get() != nullptr);
  for (auto& iter : bg_comp_workers_) {
    iter->memview_manager_ = manager;
  }
}

// 将 read_only_memtable 轮询放置在不同的 compworker 上.
void Compactor::AddReadOnlyTable(const std::shared_ptr<Memtable>& mem_table) {
  memTaskmtx_.lock();
  LOG_INFO("start add current_comp_index: {} bg_comp_workers: {}",
           current_comp_index_, bg_comp_workers_.size());
  assert(current_comp_index_ < bg_comp_workers_.size());
  bg_comp_workers_[current_comp_index_]->AddSStable(mem_table);
  current_comp_index_++;
  current_comp_index_ = ((current_comp_index_) % bg_comp_workers_.size());
  LOG_INFO("memtable: {} to current_comp_index_: {}", mem_table->getMemNumber(),
           current_comp_index_);
  memTaskmtx_.unlock();
}

void CompWorker::SetCompactorRef(const std::shared_ptr<Compactor>& compactor) {
  compactor_ref_ = compactor;
}

uint32_t g_comp_thread_n = 0;
std::string ThreadCompName = "compworker_";

void CompWorker::Run() {
  // io_uring 默认队列深度
  iouring_.Init(kDefaultIOUringSize);
  isRunning_ = true;
  auto current_id = g_comp_thread_n++;
  bg_thread_ = std::thread([&]() {
    auto thread_name = ThreadCompName + std::to_string(current_id);
    auto ue = ::pthread_setname_np(::pthread_self(), thread_name.c_str());
    if (ue == ERANGE) {
      LOG_INFO("pthread set name {} error", ::pthread_self());
    }
    assert(ue != ERANGE);

    while (true == isRunning_) {
      struct util::iouring::ConSumptionQueue conn;
      // 后续若收集到就绪事件, 直接在 sync_data_map_ref
      // TODO: 这里是否线程安全?
      if (bg_wait_to_sync_sstable_.empty()) {
        std::unique_lock<std::mutex> lgk(notify_mtx_);
        cond_.wait_for(lgk, std::chrono::seconds(2));
      }
      // 如果没有刷盘任务, 那么查看当前是否有需要回收的数据,
      if (bg_wait_to_sync_sstable_.empty() == true) {
        do {
          // 这里不能阻塞.
          conn = iouring_.PeekFinishQueue();
          if (conn.isEmpty()) {
            break;
          }
          auto iter =
              sync_data_map_.find(static_cast<const char*>(conn.getData()));
          if (iter != sync_data_map_.end()) {
            // 对 level merge 的数据进行特殊处理.
            if (iter->second.isMajorCompact == true) {
              //
              auto ue = iter->second.sstable_ref->InitMmap();
              //
              if (false == ue) {
                LOG_ERROR(
                    "level merge sstable memtable number: {} name: {} mmap "
                    "error",
                    iter->second.memtable_n,
                    iter->second.sstable_ref->getfileName());
                assert(false);
              }
              this->memview_manager_->PushTableMergeView(
                  iter->second.sstable_ref->getNumber(),
                  iter->second.sstable_ref->getMmapPtr(),
                  iter->second.sstable_ref->getFileSize(),
                  iter->second.level_info.first,
                  iter->second.level_info.second,
                  iter->second.sstable_ref);
              sync_data_map_.erase(iter);
              iouring_.DeleteEvent(&conn);
            }
            /*
              io_uring 将数据刷入了磁盘
              构造 MemTable_view push 到 MemTableWorker 中.
            */
            auto ue = iter->second.sstable_ref->InitMmap();
            if (false == ue) {
              LOG_ERROR("memtable: {} name: {} mmap error",
                        iter->second.memtable_n,
                        iter->second.sstable_ref->getfileName());
              assert(false);
            }
            this->memview_manager_->PushTableView(
                iter->second.sstable_ref->getNumber(),
                iter->second.sstable_ref->getMmapPtr(),
                iter->second.sstable_ref->getFileSize(),
                iter->second.sstable_ref);
            LOG_TRACE("mem_view_table: {} name: {} mmap finish",
                      iter->second.sstable_ref->getNumber(),
                      iter->second.sstable_ref->getfileName());
            this->shared_memtable_ref_->PushRemoveReadonlyMemtableContext(
                iter->second.sstable_ref->getNumber());
            LOG_INFO("erase: {}", iter->first);
            sync_data_map_.erase(iter);
            iouring_.DeleteEvent(&conn);
          }
        } while (true);
        continue;
      }
      // 快速交换, 以免阻塞
      {
        task_mtx_.lock();
        std::swap(bg_wait_to_sync_sstable_, wait_to_sync_sstable_);
        task_mtx_.unlock();
      }

      LOG_INFO("compaction start to excute, wait_to_sync_sstable_size: {}",
               wait_to_sync_sstable_.size());

      for (auto& iter : wait_to_sync_sstable_) {
        // TODO: 考虑对于每次刷入到 SSTable 中, 我们应该使用 BTree 来维护一个
        // read_only_index,
        //  查找顺序 => memory_memtable_
        //         => read_only_memtable_
        //         => btree_index
        //         => sstable.

        // 这里会直接让 memtable 引用技术 = 1.
        comp_kv_data_str_ = std::make_shared<std::vector<char>>();
        //
        auto mem_table_data_ref = iter->getMemTableRef();
        // bloom 过滤器的数据
        auto mem_bloom_filter_data_ref = iter->getFilterData();

        auto mem_bloom_filter_var_size =
            varintLength(mem_bloom_filter_data_ref->getSize());

        auto mem_bloom_filter_seed_var_size =
            varintLength(iter->getBloomSeed());

        // 元数据
        // 当前版本
        auto cur_version = CompWorker::cur_version_;
        auto cur_version_var_size = varintLength(cur_version);
        // 创建时间
        auto time_now_date = TimeStamp::Now().getNowU64();
        auto time_now_date_var_size = varintLength(time_now_date);
        // 当前所对应的层级
        auto cur_level = iter->getCompactionN();
        auto cur_level_var_size = varintLength(cur_level);
        // 当前所对应的memtable号码
        auto cur_number = iter->getMemNumber();
        auto cur_number_var_size = varintLength(cur_number);
        // 所有的 key size
        auto all_key_size = iter->getMemTableRef().size();
        auto all_key_size_var_size = varintLength(all_key_size);
        //
        // bloom_filter_seed | bloom_filter_var_size | bloom_filter_data
        fmt::format_to(std::back_inserter(*comp_kv_data_str_),
                       "{}{}{}{}{}{}{}{}", format32_vec[cur_version_var_size],
                       format64_vec[time_now_date_var_size],
                       format32_vec[cur_level_var_size],
                       format32_vec[cur_number_var_size],
                       format32_vec[all_key_size_var_size],
                       format64_vec[mem_bloom_filter_seed_var_size],
                       format32_vec[mem_bloom_filter_var_size],
                       std::string_view(mem_bloom_filter_data_ref->getData(),
                                        mem_bloom_filter_data_ref->getSize()));
        assert(comp_kv_data_str_->size() >
               mem_bloom_filter_data_ref->getSize());
        // 写入该文件的版本
        auto end_ptr =
            encodeVarint32(comp_kv_data_str_->data(), CompWorker::cur_version_);
        // 创建时间
        end_ptr = encodeVarint64(end_ptr, time_now_date);
        // 对应的 level
        end_ptr = encodeVarint32(end_ptr, cur_level);
        // 对应的 号码
        end_ptr = encodeVarint32(end_ptr, cur_number);
        // 当前 memtable 的 sum of key
        end_ptr = encodeVarint32(end_ptr, all_key_size_var_size);
        // bloom_filter 的随机数种子
        end_ptr = encodeVarint64(end_ptr, iter->getBloomSeed());
        // 给 bloom_filter_varint_size 赋值
        encodeVarint32(end_ptr, mem_bloom_filter_data_ref->getSize());
        // copy bloom_filter_varint_元数据.
        // 这里不能进行压缩, 必须全量存储.
        // bloom_filter_mmap 做映射.
        //
        //
        for (auto& iter : mem_table_data_ref) {
          auto sstable_style_iter = formatMemTableToSSTableStr(iter);
          // key 存在, value 则也要存储
          // key_varint32_size
          auto varint_key_size =
              varintLength(sstable_style_iter.key_view.size());
          // value_varint32_size
          auto varint_value_size =
              varintLength(sstable_style_iter.value_view.size());
          //
          //
          auto before_insert_size = comp_kv_data_str_->size();
          //
          //
          if (sstable_style_iter.isExist == true) {
            fmt::format_to(
                std::back_inserter(*comp_kv_data_str_), "{}{}{}{}",
                format32_vec[varint_key_size], sstable_style_iter.key_view,
                format32_vec[varint_value_size], sstable_style_iter.value_view);
            // 找到之前的下标
            auto str_ptr = comp_kv_data_str_->data() + before_insert_size;
            // key 赋值
            str_ptr =
                encodeVarint32(str_ptr, sstable_style_iter.key_view.size());
            str_ptr += sstable_style_iter.key_view.size();
            str_ptr =
                encodeVarint32(str_ptr, sstable_style_iter.value_view.size());
            str_ptr += sstable_style_iter.value_view.size();
            // key 不存在, value 直接不用存储
          } else if (sstable_style_iter.isExist == false) {
            fmt::format_to(std::back_inserter(*comp_kv_data_str_), "{}{}",
                           format32_vec[varint_key_size],
                           sstable_style_iter.key_view);
            auto str_ptr = comp_kv_data_str_->data() + before_insert_size;
            encodeVarint32(str_ptr, sstable_style_iter.key_view.size());
          }
        }
        LOG_INFO("compaction ok comp_data_size: {}", comp_kv_data_str_->size());
        // 格式化完成
        /*
         minor_compaction
        */
        //
        auto current_memtable_level = iter->getCompactionN();
        auto current_memtable_number = iter->getMemNumber();
        //
        LOG_INFO("current_level: {} current_number: {}", current_memtable_level,
                 current_memtable_number);
        assert(sstable_manager_.get() != nullptr);
        auto cu_sstable = sstable_manager_->newSSTable(current_memtable_number,
                                                       current_memtable_level);
        //
        assert(cu_sstable.get() != nullptr);
        auto cu_sstable_name =
            fmt::format("memtable_{}_level_{}.sst", current_memtable_number,
                        current_memtable_level);
        // 初始化当前 sstable
        auto re =
            cu_sstable->Init(path, cu_sstable_name, current_memtable_level,
                             current_memtable_number);
        // 设置当前 sstable 的序号
        cu_sstable->setNumber(iter->getMemNumber());
        // 设置当前 sstable 的层级
        // 即第几次被 compaction.
        cu_sstable->setLevel(iter->getCompactionN());
        if (false == re) {
          LOG_ERROR("sstable init {} error", cu_sstable_name);
          assert(false);
        }
        LOG_INFO("sstable init {} ok begin write", cu_sstable_name);
        util::iouring::WriteRequest write_request(cu_sstable->getFd(),
                                                  (comp_kv_data_str_->data()),
                                                  comp_kv_data_str_->size());
        assert(compactor_ref_.get() != nullptr);
        // 需要让 sync_data_map 帮助我们管理 sync 的数据
        sync_data_map_[comp_kv_data_str_->data()] = {
            comp_kv_data_str_, iter->getMemNumber(), cu_sstable};
        iouring_.PrepWrite(&write_request);
        // 提交
        iouring_.Submit();
      }
      //
      wait_to_sync_sstable_.clear();
      //
      //
      uint32_t j = 0;
      for (; j < memtable_N_; j++) {
        // 如果当前的memtable 的层数 > 最大层数
        // 则要进行 level merge
        if (shared_memtable_ref_->getMemTaskWorkerLevel(j) > max_level_n_) {
          auto cur_memtable_view_vec = memview_manager_->getMemNVec(j);
          uint32_t l = 0;
          uint32_t cur_mem_view_size =
              cur_memtable_view_vec->memtable_vec_.size() - 1;
          // 如果成功或者当前 memtable_view_vec 的锁

          if (true == cur_memtable_view_vec->getCompactLock()) {
            LOG_INFO("start to compact merge");
            for (; l < cur_mem_view_size; l++) {
              // first_view_iter 表示相对于的 第一层的 sstable 数据
              CompData comp_data;
              comp_data.sync_data = std::make_shared<std::vector<char>>();
              // 表示该数据来自 level_merge
              comp_data.isMajorCompact = true;
              comp_data.level_info.first = l;
              comp_data.level_info.second = l + 1;
              // 编号
              comp_data.memtable_n =
                  cur_memtable_view_vec->memtable_vec_[l]->getNumber();
              // level merge 元数据.
              auto new_bloom_filter = std::make_shared<util::BloomFilter<>>();
              auto first_view_iter = cur_memtable_view_vec->memtable_vec_[l]
                                         ->getMemViewPtr()
                                         ->begin();
              // second_view_iter 表示相对于的 第二层的 sstable 数据
              auto second_view_iter =
                  cur_memtable_view_vec->memtable_vec_[l + 1]
                      ->getMemViewPtr()
                      ->begin();
              auto first_end = cur_memtable_view_vec->memtable_vec_[l]
                                   ->getMemViewPtr()
                                   ->end();
              auto second_end = cur_memtable_view_vec->memtable_vec_[l + 1]
                                    ->getMemViewPtr()
                                    ->end();
              // 合并之后的所有的 key 的数量
              uint32_t memtable_key_size = 0;
              LOG_INFO("here?");
              while (first_view_iter != first_end &&
                     second_view_iter != second_end) {
                // 如果相等直接丢弃 第二层的 iter
                auto first_key_view =
                    getMemTableViewKeyViewIter(first_view_iter);
                auto second_key_view =
                    getMemTableViewKeyViewIter(second_view_iter);
                if (first_key_view > second_key_view) {
                  fmt::format_to(std::back_inserter(*comp_data.sync_data), "{}",
                                 *second_view_iter);
                  second_view_iter++;
                  new_bloom_filter->Insert(*second_view_iter);
                } else if (first_key_view < second_key_view) {
                  fmt::format_to(std::back_inserter(*comp_data.sync_data), "{}",
                                 *first_view_iter);
                  first_view_iter++;
                  new_bloom_filter->Insert(*first_view_iter);
                  // 两个 key 相等, 取上一层的为结果
                } else {
                  fmt::format_to(std::back_inserter(*comp_data.sync_data), "{}",
                                 *first_view_iter);
                  new_bloom_filter->Insert(*first_view_iter);
                  first_view_iter++;
                  second_view_iter++;
                }
                memtable_key_size++;
              }
              // 合并两个 level, 对于 bloom_filter 则也要进行合并
              // 当合并了
              // 完成 memtable 当前的两个 level 的merge
              // level 以第一个为主
              //
              LOG_INFO("here?");
              auto cu_memtable_number =
                  shared_memtable_ref_->getMemTaskWorkerNumber(j);
              auto cu_sstable =
                  sstable_manager_->newCompactionSSTable(cu_memtable_number, l);
              assert(nullptr != cu_sstable.get());
              //
              auto cu_sstable_compact_name = fmt::format(
                  "memtable_merge_{}_merge_{}_{}.sst",
                  shared_memtable_ref_->getMemTaskWorkerNumber(j), l, l + 1);
              auto ue = cu_sstable->Init(path, cu_sstable_compact_name, l,
                                         cu_memtable_number);
              assert(ue == true);
              assert(memtable_key_size > 0);
              auto memtable_key_size_var_size = varintLength(memtable_key_size);
              auto cur_version = CompWorker::cur_version_;
              auto cur_version_var_size = varintLength(cur_version);
              auto time_now_date = TimeStamp::Now().getNowU64();
              auto cur_level_first_var_size = varintLength(l);
              auto cur_level_second_var_size = varintLength(l + 1);
              auto time_now_date_var_size = varintLength(time_now_date);
              auto cur_memtable_number_var_size =
                  varintLength(cu_memtable_number);
              auto bloom_filter_size = new_bloom_filter->getBitSetSize();
              auto bloom_filter_var_size = varintLength(bloom_filter_size);
              auto bloom_filter_seed = new_bloom_filter->getFilterSeed();
              auto bloom_filter_seed_var_size = varintLength(bloom_filter_seed);
              auto meta_data_ptr = comp_data.sync_data->data();
              fmt::format_to(std::back_inserter(*comp_data.sync_data),
                             "{}{}{}{}{}{}{}{}",
                             format32_vec[cur_version_var_size],
                             format64_vec[time_now_date_var_size],
                             format32_vec[cur_level_first_var_size],
                             format32_vec[cur_level_second_var_size],
                             format32_vec[cur_memtable_number_var_size],
                             format32_vec[memtable_key_size_var_size],
                             format32_vec[bloom_filter_var_size],
                             format64_vec[bloom_filter_seed_var_size],
                             std::string_view(
                                 new_bloom_filter->getFilterData()->getData(),
                                 new_bloom_filter->getFilterData()->getSize()));
              // 版本信息
              auto end_ptr = encodeVarint32(meta_data_ptr, cur_version);
              // 创建时间
              end_ptr = encodeVarint64(end_ptr, time_now_date);
              // 第一层被合并的 lev
              end_ptr = encodeVarint32(end_ptr, l);
              // 第二层被合并的 lev
              end_ptr = encodeVarint32(end_ptr, l + 1);
              // 内存表 number
              end_ptr = encodeVarint32(end_ptr, cu_memtable_number);
              // 合并之后的所有的 key size
              end_ptr = encodeVarint32(end_ptr, memtable_key_size);
              // 元数据的长度
              // 这里我们无法先知道两层 level 合并之后的 key 的数量
              // 但由于我们默认的还是要先写元数据
              auto meta_data_size =
                  ((comp_data.sync_data->data() + comp_data.sync_data->size()) -
                   meta_data_ptr);
              // TODO: 这里为了由于要统计上 key 的信息, 事实上, 我们不知道两层
              // level合并之后 的keyval 整体信息. 所以 ....
              // 这里只能将元数据放在后面了
              util::iouring::WriteRequest write_request_meta_data(
                  cu_sstable->getFd(), meta_data_ptr,
                  meta_data_size);
              util::iouring::WriteRequest write_request_kv_data(
                cu_sstable->getFd(), comp_data.sync_data->data(),
                (meta_data_ptr - comp_data.sync_data->data())
              );
              assert(compactor_ref_.get() != nullptr);
              sync_data_map_[meta_data_ptr] = comp_data;
              // 先写入元数据
              iouring_.PrepWriteLink(&write_request_meta_data);
              // 再写入kv
              iouring_.PrepWrite(&write_request_kv_data);
              iouring_.Submit();
            }
            cur_memtable_view_vec->finishCompact();
            l++;
          }
        }

        //
      }
    }
  });
}

}  // namespace db

}  // namespace fver