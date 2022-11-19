#include "src/db/compactor.hpp"

#include <cstring>
#include <iterator>

#include "src/base/log/logging.hpp"
#include "src/db/key_format.hpp"
#include "src/db/memtable.hpp"
#include "src/db/request.hpp"
#include "src/db/sstable_manager.hpp"
#include "src/util/iouring.hpp"

namespace fver {

namespace db {

void CompWorker::AddSStable(const std::shared_ptr<Memtable>& memtable) {
  task_mtx_.lock();
  bg_wait_to_sync_sstable_.push_back(memtable);
  task_mtx_.unlock();
  cond_.notify_one();
}

void Compactor::Init(const uint32_t size, const uint32_t worker_size,
                     const std::string& db_path) {
  // sstable 的目录
  db_path_ = db_path;
  memtable_n_ = size;
  task_workers_n_ = worker_size;
  LOG_INFO("size: {} worker_size: {}", size, worker_size);
  current_comp_index_ = 0;
  // 初始化 CompactorWorker
  bg_comp_workers_.resize(worker_size);
  for (auto& iter : bg_comp_workers_) {
    iter = std::make_shared<CompWorker>();
  }
  //
  LOG_INFO("bg_comp_worker_size: {}", bg_comp_workers_.size());
  // 初始化 sstable_manager
  sstable_manager_ = std::make_shared<SSTableManager>();
  //
  sstable_manager_->Init(memtable_n_, db_path);
  for (auto& iter : bg_comp_workers_) {
    iter->sstable_manager_ = sstable_manager_;
    iter->path = db_path;
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
  // io_uring 默认队列深度 32层
  iouring_.Init(kDefaultIOUringSize);
  isRunning_ = true;
  auto current_id = g_comp_thread_n++;
  bg_thread_ = std::thread([&]() {
    auto thread_name = ThreadCompName + std::to_string(current_id);
    auto ue = ::pthread_setname_np(::pthread_self(), thread_name.c_str());
    assert(ue != ERANGE);

    while (true == isRunning_) {
      struct util::iouring::ConSumptionQueue conn;
      // 后续若收集到就绪事件, 直接在sync_data_map_ref
      do {
        conn = iouring_.PeekFinishQueue();
        if (conn.isEmpty()) {
          break;
        }
        auto iter =
            sync_data_map_.find(static_cast<const char*>(conn.getData()));
        if (iter != sync_data_map_.end()) {
          /*
            io_uring 将数据刷入了磁盘
            // 构造 MemTable_view push 到 MemTableWorker 中.
            
          */
          sync_data_map_.erase(iter);
          iouring_.DeleteEvent(&conn);
        }
      } while (true);
      // TODO: 这里是否线程安全?
      if (bg_wait_to_sync_sstable_.empty()) {
        std::unique_lock<std::mutex> lgk(notify_mtx_);
        cond_.wait(lgk);
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

        // bloom_filter_seed | bloom_filter_var_size | bloom_filter_data
        fmt::format_to(std::back_inserter(*comp_kv_data_str_), "{}{}{}",
                       format64_vec[mem_bloom_filter_seed_var_size],
                       format32_vec[mem_bloom_filter_var_size],
                       std::string_view(mem_bloom_filter_data_ref->getData(),
                                        mem_bloom_filter_data_ref->getSize()));
        assert(comp_kv_data_str_->size() >
               mem_bloom_filter_data_ref->getSize());
        // bloom_filter seed 赋值
        auto end_ptr =
            encodeVarint64(comp_kv_data_str_->data(), iter->getBloomSeed());

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
            fmt::format("memtable_{}_level{}.sst", current_memtable_number,
                        current_memtable_level);
        //
        LOG_INFO("memtable: {} level: {} ok", current_memtable_number,
                 current_memtable_level);
        // 初始化当前 sstable
        auto re =
            cu_sstable->Init(path, cu_sstable_name, current_memtable_level,
                             current_memtable_number);
        if (false == re) {
          LOG_ERROR("sstable init {} error", cu_sstable_name);
          assert(false);
        }
        LOG_INFO("sstable init {} ok begin write", cu_sstable_name);
        util::iouring::WriteRequest write_request(cu_sstable->getFd(),
                                                  (comp_kv_data_str_->data()),
                                                  comp_kv_data_str_->size());
        LOG_INFO("sync data: {} data_use_count: {} comp_kv_size: {}",
                 std::string_view(comp_kv_data_str_->data(),
                                  comp_kv_data_str_->size()),
                 comp_kv_data_str_.use_count(), comp_kv_data_str_->size());
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
    }
  });
}

}  // namespace db

}  // namespace fver