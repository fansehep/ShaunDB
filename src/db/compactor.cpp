#include "src/db/compactor.hpp"

#include <cstring>
#include <iterator>

#include "src/base/log/logging.hpp"
#include "src/db/key_format.hpp"
#include "src/db/memtable.hpp"
#include "src/db/request.hpp"
#include "src/db/sstable_manager.hpp"

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
  uint32_t i = 0;
  // 初始化 CompactorWorker
  bg_comp_workers_.resize(worker_size);
  comp_data_map_.resize(memtable_n_);
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
  for (auto& iter : bg_comp_workers_) {
    iter->SetCompactorRef(compactor);
  }
}

// 将 read_only_memtable 轮询放置在不同的 compworker 上.
void Compactor::AddReadOnlyTable(const std::shared_ptr<Memtable>& mem_table) {
  memTaskmtx_.lock();
  LOG_INFO("start add current_comp_index: {} bg_comp_workers: {}",
           current_comp_index_, bg_comp_workers_.size());
  bg_comp_workers_[current_comp_index_]->AddSStable(mem_table);
  current_comp_index_++;
  current_comp_index_ = ((current_comp_index_) % bg_comp_workers_.size());
  LOG_INFO("memtable: {} to current_comp_index_: {}", mem_table->getMemNumber(),
           current_comp_index_);
  memTaskmtx_.unlock();
}

void Compactor::AddSyncData(const std::shared_ptr<std::vector<char>>& data,
                            const int n) {
  mtx_.lock();
  //
  LOG_INFO("compactor n: {}", n);
  comp_data_map_[n].push_back(data);
  mtx_.unlock();
}

void CompWorker::SetCompactorRef(const std::shared_ptr<Compactor>& compactor) {
  compactor_ref_ = compactor;
}

void CompWorker::Run() {
  // io_uring 默认队列深度 32层
  iouring_.Init(kDefaultIOUringSize);
  isRunning_ = true;
  bg_thread_ = std::thread([&]() {
    while (true == isRunning_) {
      if (wait_to_sync_sstable_.empty()) {
        std::unique_lock<std::mutex> lgk(notify_mtx_);
        cond_.wait(lgk);
      }

      // 快速交换, 以免阻塞
      {
        task_mtx_.lock();
        std::swap(bg_wait_to_sync_sstable_, wait_to_sync_sstable_);
        task_mtx_.unlock();
      }

      LOG_INFO("compaction start to excute");

      for (auto& iter : wait_to_sync_sstable_) {
        // TODO: 考虑对于每次刷入到 SSTable 中, 我们应该使用 BTree 来维护一个
        // read_only_index,
        //  查找顺序 => memory_memtable_
        //         => read_only_memtable_
        //         => btree_index
        //         => sstable.

        // 这里会直接让 memtable 引用技术 = 0.
        comp_kv_data_str_ = std::make_shared<std::vector<char>>();
        //
        auto mem_table_data_ref = iter->getMemTableRef();
        // bloom 过滤器的数据
        auto mem_bloom_filter_data_ref = iter->getFilterData();

        auto mem_bloom_filter_var_size =
            varintLength(mem_bloom_filter_data_ref.getSize());

        // bloom_filter_var_size | bloom_filter_data
        fmt::format_to(std::back_inserter(*comp_kv_data_str_), "{}{}",
                       format32_vec[mem_bloom_filter_var_size],
                       std::string_view(mem_bloom_filter_data_ref.getData(),
                                        mem_bloom_filter_data_ref.getSize()));
        //
        // 给 bloom_filter_varint_size 赋值
        encodeVarint32((*comp_kv_data_str_).data(),
                       mem_bloom_filter_data_ref.getSize());
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
                                                  comp_kv_data_str_->data(),
                                                  comp_kv_data_str_->size());
        LOG_INFO("sync data: {}", std::string_view(comp_kv_data_str_->data(),
                                                   comp_kv_data_str_->size()));
        compactor_ref_->AddSyncData(comp_kv_data_str_, iter->getMemNumber());
        LOG_INFO("iouring prep write request");
        iouring_.PrepWrite(&write_request);
        // 提交
        iouring_.Submit();
      }
      //
      // 清理后台需要刷新的 table
      wait_to_sync_sstable_.clear();
    }
  });
}

}  // namespace db

}  // namespace fver