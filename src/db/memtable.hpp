#ifndef SRC_DB_MEMTABLE_H_
#define SRC_DB_MEMTABLE_H_

#include <memory>
#include <cstdio>
#include <map>
#include <mutex>
#include "src/db/request.hpp"
#include "src/util/bloomfilter.hpp"
#include "src/parallel_hashmap/btree.h"

namespace fver {
namespace db {

class Memtable {
public:
  Memtable();
  ~Memtable() = default;

private:
  // 是否只是可读
  // 当一个 Memtable 写到一定容量之时, 便应该成为一个
  // ImmuTable, 等待后台线程做 Minor_Compaction.
  // false 当前还没有写满
  // true 即可变成
  bool isReadonly_;

  // 使用读写锁, 来保证单个 btree 的线程安全
  std::shared_mutex shd_mtx_;

  // 

  // 存储数据的内存表, 使用 phmap::btree_set 来使用
  std::unique_ptr<phmap::btree_set<std::string>> memMap_;
  util::BloomFilter<> bloomFilter_;
};

}

}


#endif