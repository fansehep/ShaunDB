#ifndef SRC_DB_MEMTABLE_H_
#define SRC_DB_MEMTABLE_H_

#include <absl/container/btree_set.h>

#include <cstdio>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <shared_mutex>

#include "src/db/key_format.hpp"
#include "src/db/request.hpp"
#include "src/util/bloomfilter.hpp"

namespace fver {
namespace db {

// 默认一个 Memtable 的最大容量是 128 MB
// 超过该容量即可变成 一个 read_only_memtable
static constexpr uint32_t kDefaultMemtableSize = 128 * 1024 * 1024;

class Memtable {
 public:
  Memtable();
  ~Memtable() = default;

  void Set(const std::shared_ptr<SetContext> set_context);

  void Get(const std::shared_ptr<GetContext> get_context);

  void Delete(const std::shared_ptr<DeleteContext> del_context);

  // 获取当前 memMap 的所占内容容量
  uint32_t getMemSize();

  MemBTree getMemTable();

  // 让当前 Memtable 只读
  void setReadOnly();

  // 设置 Memtable 的编号
  void setNumber(const uint32_t number);

  // 获取当前内存表的编号
  uint32_t getMemNumber() const;

  // 增加一次引用计数
  void addRefs();

  // 获取引用计数
  uint32_t getRefs();

  // 减少引用计数
  void decreaseRefs();

  // 返回 bloom_filter 数据
  auto& getFilterData() { return bloomFilter_.getFilterData(); }

 private:
  // 当前内存表所花费的内存
  uint32_t memSize_;

  // 是否只是可读
  // 当一个 Memtable 写到一定容量之时, 便应该成为一个
  // ImmuTable, 等待后台线程做 Minor_Compaction.
  // false 当前还没有写满
  // true 即可变成
  bool isReadonly_;

  // 当前 memtable 的编号
  uint64_t memtable_number_;

  // memtable 当写到固定阈值的时候
  // 会成为一个 read_only_memtable
  // 但是查找的时候, 可能会去从 read_only_memtable
  // 为了在合适的时候刷入, 需要自动维护一个引用计数
  std::atomic<uint32_t> refs_;

  // 存储数据的内存表
  MemBTree memMap_;
  // TODO: 当前使用 btree 当作内存表, b tree 的查询速率应该算是很快了
  // 是否应该考虑去掉 bloomFilter_, 而且这里
  util::BloomFilter<> bloomFilter_;
};

}  // namespace db

}  // namespace fver

#endif