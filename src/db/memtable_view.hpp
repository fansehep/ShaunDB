#ifndef SRC_DB_MEMTABLE_VIEW_H_
#define SRC_DB_MEMTABLE_VIEW_H_

#include "src/db/key_format.hpp"
#include "src/db/request.hpp"
#include "src/util/bloomfilter.hpp"

namespace fver {

namespace db {

class SSTable;

struct CompactInfo {
  // 第一层 merge 的层 number
  uint32_t firstLevel;
  // 第二层 merge 的层 number
  uint32_t secondLevel;
  // 当 Compactor 完成 mino Compaction 之后
  // MemTaskWorker 需要将 合并之后的结果
  // 替换掉原有的2个 memtable_view
  bool IsInstead;
};

// readonly
class MemTable_view {
 public:
  //
  MemTable_view() : isReadable_(false) {}
  ~MemTable_view();
  //

  MemTable_view(const char* data, const uint32_t data_size, const uint32_t n,
                const uint32_t lev, const std::shared_ptr<SSTable>& sstable);

  //
  MemTable_view(const char* data,
                const uint32_t data_size,
                const uint32_t number,
                const uint32_t level_1,
                const uint32_t level_2,
                const std::shared_ptr<SSTable>& sstable);

  bool Init(std::string_view mmap_view);

  void Reset(const char* data, const uint32_t data_size);

  void Get(const std::shared_ptr<GetContext>& get_context);

  bool isReadAble() { return isReadable_; }

  // 当前 memTable_view 只读
  void setReadOnly() { isReadable_ = true; }

  void setNumber(const uint32_t n);

  uint32_t getNumber();

  void setLevel(const uint32_t n);

  uint32_t getLevel();

  auto getMemViewPtr() {
    return &memMapView_;
  }

  auto getCurVersion() {
    return version_;
  }

  auto getBloomFilterPtr() {
    return bloomFilter_.get();
  }

  auto getBloomFilterSize() {
    return bloomFilter_->getBitSetSize();
  }

 private:

  std::shared_ptr<SSTable> sstable_ref_;
  // compaction 的信息.
  CompactInfo info_;

  // 当前映射的 version
  uint32_t version_;

  // mmap 映射的指针
  const char* memViewPtr_;

  // mmap 映射的大小
  uint32_t memViewSize_;

  uint32_t cur_level_;

  // 当前是否可读.
  bool isReadable_;

  // 当前 memtable_view 的号码
  uint32_t number_n_;

  // std::set<std::string_view>
  MemBTreeView memMapView_;

  std::unique_ptr<util::BloomFilter<>> bloomFilter_;
};

}  // namespace db

}  // namespace fver

#endif