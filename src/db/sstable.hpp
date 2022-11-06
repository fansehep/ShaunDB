#ifndef SRC_DB_SSTABLE_H_
#define SRC_DB_SSTABLE_H_

#include "src/base/noncopyable.hpp"
#include "src/db/shared_memtable.hpp"
#include "src/util/file/appendfile.hpp"

using ::fver::base::NonCopyable;

namespace fver {

namespace db {

static constexpr uint32_t kDefaultLevelFileSize = 7;

class SSTableLevel {
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
  void Init(const std::string& sstablePath, const uint32_t level_size,
            const uint32_t memtable_number);

  /*
    关闭所有 level 文件
  */
  void Close();

  // 向 level 层级文件中进行写入
  /*
    @brief data: 数据
    @brief data_size; 数据大小
    // 由于一个 memtable 分为 7 层
    // 所以当第一个文件完成写入之后, 顺序写入, 应该向第二个文件继续写入.
  */
  void Write(char* data, const size_t data_size);

  /*
    @brief: data: 数据
    @brief: data_size; 数据长度
    @level_idx: 选择向哪一层 SSTable 中写入.
  */
  void WriteToIdx(char* data, const size_t data_size, const uint32_t level_idx);

  // 获取当前的 level_sstable 的数量
  uint32_t getCurrentMemLevelSize();

  // 想要读取哪一层的 SStable.
  void Read(std::string* str_data, const size_t level_idx);

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

  // 当前写入的 sstable.
  uint32_t current_sstable_lev_;

  // SStable 的前缀名称 memtable_0_sstable_0.sst

  // SStable 所对应的路径
  std::string sstable_path_;
};


}  // namespace db

}  // namespace fver

#endif