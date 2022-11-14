#include "src/db/compactor.hpp"

#include <cstring>
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
  comp_data_map_.resize(memtable_n_);
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

        //
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
        // 给 bloom_filter_varint_size 赋值
        encodeVarint32((*comp_kv_data_str_).data(),
                                      mem_bloom_filter_data_ref.getSize());
        // copy bloom_filter_varint_元数据.
        // 这里不能进行压缩, 必须全量存储.
        // 方便 bloom_filter_mmap 做映射.
        //
        // 由于 std::vector 会自动扩容, 所以只能通过下标来访问数据.
        auto start_idx = comp_kv_data_str_->size();
        //
        auto str_ptr = comp_kv_data_str_->data() + start_idx;
        for (auto& iter : mem_table_data_ref) {
          auto sstable_style_iter = formatMemTableToSSTableStr(iter);
          // key 存在, value 则也要存储
          // key_varint32_size
          auto varint_key_size = varintLength(sstable_style_iter.key_view.size());
          // value_varint32_size
          auto varint_value_size = varintLength(sstable_style_iter.value_view.size());

          if (sstable_style_iter.isExist == true) {
            fmt::format_to(std::back_inserter(*comp_kv_data_str_), "{}{}{}{}",
                           format32_vec[varint_key_size],
                           sstable_style_iter.key_view,
                           format32_vec[varint_key_size],
                           sstable_style_iter.value_view);
            str_ptr = encodeVarint32(str_ptr, sstable_style_iter.key_view.size());
            str_ptr += sstable_style_iter.key_view.size();
            str_ptr = encodeVarint32(str_ptr, sstable_style_iter.value_view.size());
            start_idx
            // key 不存在, value 直接不用存储
          } else if (sstable_style_iter.isExist == false) {
          }
        };
      }
    }
  });
}

}  // namespace db

}  // namespace fver