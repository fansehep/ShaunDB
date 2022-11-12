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
        // comp_meta_data_str_ =
        //     fmt::format("{}{}{}{}{}", kEmpty4Space, kEmpty4Space, kEmpty4Space,
        //                 kEmpty4Space, kEmpty4Space);
        //
        

        // 当前 memtable 最大的 key_value_view
        auto max_sstable_fmt_style =
            formatMemTableToSSTable(mem_table_data_ref.end());
        //
        // 当前 memtable 最小的 key_value_view
        auto min_sstable_fmt_style =
            formatMemTableToSSTable(mem_table_data_ref.begin());
        //
        //
        //
        //
        //
        int i = 0;
        // 复用数据
        std::vector<SSTableKeyValueStyle> sstable_key_value_vec;
        sstable_key_value_vec.resize(prefix_size_);
        for (auto& iter : mem_table_data_ref) {
          sstable_key_value_vec[i] = formatMemTableToSSTableStr(iter);

          // 统计总的内存容量
          i++;
          // sstable_key_value_vec 已经被赋值
          if (i == prefix_size_) {
            // 第一个 kv 的数据从 0 开始.
            comp_kv_index_vec_.push_back(0);
            // 当前 prefix 个的相同前缀 pre_key_view, pre_value_view.
            auto same_prefix_key_value =
                Format16PrefixStr(sstable_key_value_vec);
            // 共享 key 的 varint 编码长度
            auto same_prefix_key_varint_size =
                varintLength(same_prefix_key_value.key_view.size());
            // 共享 value 的 varint 编码长度
            auto same_prefix_value_varint_size =
                varintLength(same_prefix_key_value.value_view.size());
            auto first_shared_key_varint_size =
                varintLength(sstable_key_value_vec[0].key_view.size());
            auto first_shared_value_varint_size =
                varintLength(sstable_key_value_vec[0].value_view.size());

            // 第一个非共享字符串格式化.
            fmt::format_to(std::back_inserter(comp_kv_data_str_),
                           "{}{}{}{}{}{}",
                           format32_vec[same_prefix_key_varint_size],
                           format32_vec[first_shared_key_varint_size],
                           sstable_key_value_vec[0].key_view,
                           format32_vec[same_prefix_value_varint_size],
                           format32_vec[first_shared_value_varint_size],
                           sstable_key_value_vec[0].value_view);
            // 赋值 shared_key_size
            auto end_ptr =
                encodeVarint32(comp_kv_data_str_.data(),
                               same_prefix_key_value.key_view.size());
            // 赋值 key_size
            end_ptr = encodeVarint32(end_ptr,
                                     sstable_key_value_vec[0].key_view.size());
            // 赋值 shared_value_size
            end_ptr = encodeVarint32(
                end_ptr + sstable_key_value_vec[0].key_view.size(),
                same_prefix_key_value.value_view.size());
            // 赋值 value_size
            end_ptr = encodeVarint32(
                end_ptr, sstable_key_value_vec[0].value_view.size());
            //
            end_ptr += sstable_key_value_vec[0].value_view.size();
            int i = 1;
            for (; i < prefix_size_; i++) {
              // 当前 key_size - shared_key_size
              // 当前 value_size - shared_value_size
              auto no_shared_key_size =
                  sstable_key_value_vec[i].key_view.size() -
                  same_prefix_key_value.key_view.size();
              auto no_shared_value_size =
                  sstable_key_value_vec[i].value_view.size() -
                  same_prefix_key_value.value_view.size();
              std::string_view current_key_view(
                  sstable_key_value_vec[i].key_view.data() +
                      same_prefix_key_value.key_view.size(),
                  no_shared_key_size);
              std::string_view current_value_view(
                  sstable_key_value_vec[i].key_view.data() +
                      same_prefix_key_value.value_view.size(),
                  no_shared_value_size);
              fmt::format_to(std::back_inserter(comp_kv_data_str_), "{}{}{}{}",
                             format32_vec[varintLength(no_shared_key_size)],
                             current_key_view,
                             format32_vec[varintLength(no_shared_value_size)],
                             current_value_view);
              // 赋值 no_shared_key_size
              end_ptr = encodeVarint32(end_ptr, no_shared_key_size);
              end_ptr += current_key_view.size();
              encodeVarint32(end_ptr, no_shared_value_size);
              end_ptr += current_value_view.size();
              // 赋值 no_shared_value_size
            }
            i = 0;

            // 清除 sstable_kv_vec
            comp_kv_index_vec_.push_back(end_ptr - comp_kv_data_str_.data());
          }
        }
        // 如果 sstable_key_value_vec 不为空, 则表示还有剩下的数据.
        if (false != sstable_key_value_vec.empty()) {
          auto same_prefix_key_value = Format16PrefixStr(sstable_key_value_vec);
          // 共享 key 的 varint 编码长度
          auto same_prefix_key_varint_size =
              varintLength(same_prefix_key_value.key_view.size());
          // 共享 value 的 varint 编码长度
          auto same_prefix_value_varint_size =
              varintLength(same_prefix_key_value.value_view.size());
          auto first_shared_key_varint_size =
              varintLength(sstable_key_value_vec[0].key_view.size());
          auto first_shared_value_varint_size =
              varintLength(sstable_key_value_vec[0].value_view.size());

          // 第一个非共享字符串格式化.
          fmt::format_to(std::back_inserter(comp_kv_data_str_), "{}{}{}{}{}{}",
                         format32_vec[same_prefix_key_varint_size],
                         format32_vec[first_shared_key_varint_size],
                         sstable_key_value_vec[0].key_view,
                         format32_vec[same_prefix_value_varint_size],
                         format32_vec[first_shared_value_varint_size],
                         sstable_key_value_vec[0].value_view);
          // 赋值 shared_key_size
          auto end_ptr = encodeVarint32(comp_kv_data_str_.data(),
                                        same_prefix_key_value.key_view.size());
          // 赋值 key_size
          end_ptr =
              encodeVarint32(end_ptr, sstable_key_value_vec[0].key_view.size());
          // 赋值 shared_value_size
          end_ptr =
              encodeVarint32(end_ptr + sstable_key_value_vec[0].key_view.size(),
                             same_prefix_key_value.value_view.size());
          // 赋值 value_size
          end_ptr = encodeVarint32(end_ptr,
                                   sstable_key_value_vec[0].value_view.size());
          //
          end_ptr += sstable_key_value_vec[0].value_view.size();
          comp_kv_index_vec_.push_back(end_ptr - comp_kv_data_str_.data());
          int i = 1;
          const int N = sstable_key_value_vec.size();
          for (; i < N; i++) {
            // 当前 key_size - shared_key_size
            // 当前 value_size - shared_value_size
            auto no_shared_key_size = sstable_key_value_vec[i].key_view.size() -
                                      same_prefix_key_value.key_view.size();
            auto no_shared_value_size =
                sstable_key_value_vec[i].value_view.size() -
                same_prefix_key_value.value_view.size();
            std::string_view current_key_view(
                sstable_key_value_vec[i].key_view.data() +
                    same_prefix_key_value.key_view.size(),
                no_shared_key_size);
            std::string_view current_value_view(
                sstable_key_value_vec[i].key_view.data() +
                    same_prefix_key_value.value_view.size(),
                no_shared_value_size);
            fmt::format_to(std::back_inserter(comp_kv_data_str_), "{}{}{}{}",
                           format32_vec[varintLength(no_shared_key_size)],
                           current_key_view,
                           format32_vec[varintLength(no_shared_value_size)],
                           current_value_view);
            // 赋值 no_shared_key_size
            end_ptr = encodeVarint32(end_ptr, no_shared_key_size);
            end_ptr += current_key_view.size();
            encodeVarint32(end_ptr, no_shared_value_size);
            end_ptr += current_value_view.size();
            // 赋值 no_shared_value_size
          }
          // 最后一个 memtable 的结尾index
          comp_kv_index_vec_.push_back(end_ptr - comp_kv_data_str_.data());
        }
        // 单个memtable kv 已经全部格式化.
        comp_kv_data_str_.clear();
      }

      // 清理 str_data
    }
  });
}

}  // namespace db

}  // namespace fver