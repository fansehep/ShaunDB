#include "src/db/compactor.hpp"

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
  sstable_manager_ = std::make_shared<SSTableManager>();
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
        //
        // 元数据.

        // read_only_memtable => SSTable
        // 内存表 kv 的容量
        uint32_t memtable_size = mem_table_data_ref.size();
        // 32 + 4 + 4 + 4 + 4 = 48.
        comp_meta_data_str_ =
            fmt::format("{}{}{}{}{}", kEmpty32Space, kEmpty4Space, kEmpty4Space,
                        kEmpty4Space, kEmpty4Space);

        // 最大的 key_value_view
        auto max_sstable_fmt_style =
            formatMemTableToSSTable(mem_table_data_ref.end());
        // 最小的 key_value_view
        auto min_sstable_fmt_style =
            formatMemTableToSSTable(mem_table_data_ref.begin());
        //
        //
        //
        //
        //
        int format_idx = 32;

        int i = 0;
        // 复用数据
        std::vector<SSTableKeyValueStyle> sstable_key_value_vec;
        sstable_key_value_vec.resize(16);
        for (auto& iter : mem_table_data_ref) {
          sstable_key_value_vec[i] = formatMemTableToSSTableStr(iter);
          i++;
          if (i == 15) {
            
            i --;
          }
        }
      }

      // 清理 str_data
    }
  });
}

}  // namespace db

}  // namespace fver