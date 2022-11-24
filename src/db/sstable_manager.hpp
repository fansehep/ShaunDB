#ifndef SRC_DB_SSTABLE_MANAGER_H_
#define SRC_DB_SSTABLE_MANAGER_H_

#include <memory>
#include <vector>

#include "src/db/sstable.hpp"
#include "src/db/dbconfig.hpp"

namespace fver {

namespace db {

class SSTableManager {
 public:
  void Init(const uint32_t memtable_n, const std::string& dbpath);

  // 新创建一个 sstable.
  auto newSSTable(const uint32_t number, const uint32_t level)
      -> std::shared_ptr<SSTable>;

  //
  auto getSSTable(const uint32_t number, const uint32_t level);

  // 返回指定 number 的层数.
  auto getNumSize(const uint32_t number);

  // 返回当前有多少个 vec_sstable
  auto getVecSize();

 private:
  // 有多少个 memtable
  uint32_t memtable_n_;

  // 数据库的地址
  std::string db_path_;
  // sstable 的最大层数
  uint32_t max_sstable_level_;
  //
  std::vector<std::vector<std::shared_ptr<SSTable>>> sstable_vec_;
};

}  // namespace db

}  // namespace fver

#endif