#include "src/db/sstable_manager.hpp"

#include "src/util/file/appendfile.hpp"

namespace fver {

namespace db {

void SSTableLevel::Init(const std::string& sstable_path,
                        const uint32_t level_size,
                        const uint32_t memtable_number) {
  sstable_path_ = sstable_path;
  append_file_vec_.resize(level_size);
  // 默认先初始化第一层 AppendFile
  append_file_vec_[0] = std::make_unique<util::file::AppendFile>();
  assert(append_file_vec_[0].get() != nullptr);
  // 第一层 SSTable 所对应的文件名称
  auto append_file_first_name = fmt::format("memtable_{}_sstable_0.sst", memtable_number);
  // 初始化
  append_file_vec_[0]->Init(sstable_path_, append_file_first_name);
}







}  // namespace db

}  // namespace fver