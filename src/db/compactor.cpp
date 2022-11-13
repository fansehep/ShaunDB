#include "src/db/compactor.hpp"

#include <iterator>

#include "src/base/log/logging.hpp"
#include "src/db/key_format.hpp"
#include "src/db/memtable.hpp"
#include "src/db/sstable_manager.hpp"

namespace fver {

namespace db {

void CompWorker::AddSStable(const std::shared_ptr<Memtable>& memtable) {
  task_mtx_.lock();
  bg_wait_to_sync_sstable_.push_back(memtable);
  task_mtx_.unlock();
}

void Compactor::Init(const uint32_t size, const uint32_t worker_size,
                     const std::string& db_path) {
  // sstable 的目录
  db_path_ = db_path;
  memtable_n_ = size;
  task_workers_n_ = worker_size;
  current_comp_index_ = 0;
  int i = 0;
  // 初始化 CompactorWorker
  bg_comp_workers_.resize(worker_size);
  for (; i < memtable_n_; i++) {
    bg_comp_workers_[i] = std::make_shared<CompWorker>();
  }
  // 初始化 sstable_manager
  // sstable_manager_ = std::make_shared<SSTableManager>();
}

// 将 read_only_memtable 轮询放置在不同的 compworker 上.
void Compactor::AddReadOnlyTable(const std::shared_ptr<Memtable>& mem_table) {
  bg_comp_workers_[current_comp_index_]->AddSStable(mem_table);
  current_comp_index_ = ((current_comp_index_ + 1) % bg_comp_workers_.size());
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

      for (auto& iter : wait_to_sync_sstable_) {
        // TODO: 考虑对于每次刷入到 SSTable 中, 我们应该使用 BTree 来维护一个
        // read_only_index,
        //  查找顺序 => memory_memtable_
        //         => read_only_memtable_
        //         => btree_index
        //         => sstable.
        auto mem_table_data_ref = iter->getMemTableRef();
        // bloom 过滤器的数据
        auto mem_bloom_filter_data_ref = iter->getFilterData();

        auto mem_bloom_filter_var_size =
            varintLength(mem_bloom_filter_data_ref.getSize());

        // bloom_filter_var_size | bloom_filter_data
        fmt::format_to(std::back_inserter(comp_kv_meta_str_), "{}{}",
                       format32_vec[mem_bloom_filter_var_size],
                       std::string_view(mem_bloom_filter_data_ref.getData(),
                                        mem_bloom_filter_data_ref.getSize()));
        // 给 bloom_filter_varint_size 赋值
        auto end_ptr = encodeVarint32(comp_kv_meta_str_.data(), mem_bloom_filter_data_ref.getSize());
        std::mempcy(end_ptr, mem_bloom_filter_data_ref.getData(), )
      }
    }
  });
}

}  // namespace db

}  // namespace fver