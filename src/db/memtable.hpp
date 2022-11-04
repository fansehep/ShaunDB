#ifndef SRC_DB_MEMTABLE_H_
#define SRC_DB_MEMTABLE_H_

#include <cstdio>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <shared_mutex>

#include "src/db/comparator.hpp"
#include "src/db/request.hpp"
#include <absl/container/btree_set.h>
#include "src/util/bloomfilter.hpp"

namespace fver {
namespace db {

class Memtable {
 public:
  Memtable();
  ~Memtable() = default;

  void Set(const std::shared_ptr<SetContext> set_context);

  void Get(const std::shared_ptr<GetContext> get_context);

  void Delete(const std::shared_ptr<DeleteContext> del_context);


  uint32_t getMemSize();

  absl::btree_set<std::string, Comparator, std::allocator<std::string>>& getMemTable();
 
  // 让当前 Memtable 只读
  void SetReadOnly();

 private:

  
  // 是否只是可读
  // 当一个 Memtable 写到一定容量之时, 便应该成为一个
  // ImmuTable, 等待后台线程做 Minor_Compaction.
  // false 当前还没有写满
  // true 即可变成
  bool isReadonly_;


  // 最大容量
  // 当超过该容量时, 即变成 不可写入状态, 等待被 compaction
  uint32_t maxSize_;

  // 存储数据的内存表
  absl::btree_set<std::string, Comparator, std::allocator<std::string>> memMap_;
  // TODO: 当前使用 btree 当作内存表, b tree 的查询速率应该算是很快了
  // 是否应该考虑去掉 bloomFilter_, 而且这里
  util::BloomFilter<> bloomFilter_;
};

}  // namespace db

}  // namespace fver

#endif