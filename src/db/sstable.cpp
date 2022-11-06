#include "src/db/sstable.hpp"

#include "src/util/file/appendfile.hpp"

namespace fver {

namespace db {

void SSTableLevel::Init(const std::string& sstable_path,
                        const uint32_t level_size,
                        const uint32_t memtable_number) {
  sstable_path_ = sstable_path;
  // 初始化之时, current_sstable_lev 应该是 0
  current_sstable_lev_ = 0;
  //
  // LevelSStable 对应哪一个 Memtable
  memtable_seqeunce_number_ = memtable_number;
  append_file_vec_.resize(level_size);
  // 默认先初始化第一层 AppendFile
  append_file_vec_[0] = std::make_unique<util::file::AppendFile>();
  assert(append_file_vec_[0].get() != nullptr);
  // 第一层 SSTable 所对应的文件名称
  auto append_file_first_name =
      fmt::format("memtable_{}_sstable_0.sst", memtable_number);
  // 初始化
  append_file_vec_[0]->Init(sstable_path_, append_file_first_name);
  LOG_TRACE("file: {} has been init", append_file_first_name);
}

void SSTableLevel::Write(char* data, const size_t data_size) {
  // 一次写入,
  append_file_vec_[current_sstable_lev_]->Append(data, data_size);
  // 将 buf 中的数据也全部刷入到 文件中.
  append_file_vec_[current_sstable_lev_]->FlushBuffer();
  //
  LOG_TRACE("memtable: {} level_sst: {} write to file",
            memtable_seqeunce_number_, current_sstable_lev_);
  current_sstable_lev_++;
  // 为下一次写入初始化 level_sstable.sst 文件
  append_file_vec_[current_sstable_lev_] =
      std::make_unique<util::file::AppendFile>();
  // 下一层的文件名称
  auto current_append_file_name =
      fmt::format("memtable_{}_sstable_{}.sst", memtable_seqeunce_number_,
                  current_sstable_lev_);
  // 初始化下一层要写的文件
  append_file_vec_[current_sstable_lev_]->Init(sstable_path_,
                                               current_append_file_name);
}

void SSTableLevel::WriteToIdx(char* data, const size_t data_size,
                              const uint32_t level_idx) {
  assert(level_idx <= 7);
  append_file_vec_[level_idx]->Append(data, data_size);
  append_file_vec_[level_idx]->FlushBuffer();
  LOG_TRACE("write to mem: {} sstable: {} data_size: {}", memtable_seqeunce_number_,
            level_idx, data_size);
}

uint32_t SSTableLevel::getCurrentMemLevelSize() { return current_sstable_lev_; }

void SSTableLevel::Close() {
  int i = 0;
  for (; i < current_sstable_lev_; i++) {
    append_file_vec_[i]->Close();
  }
}

void SSTableLevel::Read(std::string* str_data, const size_t level_idx) {
  assert(level_idx <= 7);
  return append_file_vec_[level_idx]->Read(str_data);
}

}  // namespace db

}  // namespace fver