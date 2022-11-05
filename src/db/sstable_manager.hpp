#ifndef SRC_DB_SSTABLE_MANAGER_H_
#define SRC_DB_SSTABLE_MANAGER_H_

#include "src/base/noncopyable.hpp"
#include "src/db/shared_memtable.hpp"
#include "src/util/file/appendfile.hpp"

using ::fver::base::NonCopyable;

namespace fver {

namespace db {

static constexpr uint32_t kDefaultLevelFileSize = 7;

class SSTableLevel : public NonCopyable {
 public:
  // level 文件数量
  SSTableLevel() = default;
  ~SSTableLevel() = default;
  // 默认一个 Memtable 所拥有的文件层级是 7 层.

  /*
   * @sstable_path: 存放 SStable 的目录
   * @file_size 一个  SStable_Level 有多少层
   * @memtable_number: 当前 SSTableLevel 所对应的 序列号
   */
  void Init(const std::string& sstablePath,
            const uint32_t level_size,
            const uint32_t memtable_number);
  void Close();

  // 向 level 层级文件中进行写入
  void Write(const char* data, const size_t data_size);


 private:
  // 单个默认 Memtable 对应 7 个 Level 文件
  uint32_t level_size;

  // Memtable 所对应的层级文件的抽象
  std::vector<std::unique_ptr<util::file::AppendFile>> append_file_vec_;

  //
  // 一个 memtable 对应一个 SStableLevel
  // 由于对应多层 SStable, 所以 每个文件的名称是
  // : memtable_{memtable_sequence_number}_sstable_{number}.sst;
  //
  uint32_t memtable_seqeunce_number_;

  // SStable 的前缀名称 memtable_0_sstable_0.sst

  // SStable 所对应的路径
  std::string sstable_path_;
};

// 每个 Memtable 对应多个 LevelSStable;
class SSTableManager : public NonCopyable {
 public:
 private:
};

}  // namespace db

}  // namespace fver

#endif