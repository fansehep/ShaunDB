#ifndef SRC_DB_SSTABLE_MANAGER_H_
#define SRC_DB_SSTABLE_MANAGER_H_

#include <absl/container/btree_map.h>

#include <memory>

#include "src/base/log/logging.hpp"
#include "src/base/noncopyable.hpp"
#include "src/db/memtable.hpp"
#include "src/db/sstable.hpp"

using ::fver::base::NonCopyable;

namespace fver {

namespace db {

// 对于 SStable 一组的管理
// 每个 Memtable 对应一个 SStable.
class SSTableManager : public NonCopyable {
 public:
  SSTableManager() = default;
  void Init(const uint32_t MemTableN, const std::string& sstable_path,
            const uint32_t level_size);

 private:
  // 每个 memtable 对应一个 sstable
  uint32_t current_memtable_n_;
  //
  std::vector<SSTableLevel> single_table_vec_;

  //
};

}  // namespace db

}  // namespace fver

#endif