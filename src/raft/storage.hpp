#ifndef SRC_RAFT_STORAGE_H_
#define SRC_RAFT_STORAGE_H_

#include <rocksdb/db.h>
#include <rocksdb/slice.h>
#include <rocksdb/status.h>

#include <memory>
#include <string_view>

#include "src/base/noncopyable.hpp"

namespace fver {

namespace raft {

using status = rocksdb::Status;
using NonCopyable = fver::base::NonCopyable;

class RaftStorage final : public NonCopyable {
 public:
  RaftStorage() = default;

  status init();

  /**
   * @brief: put key value to rocksdb
   */
  status put(const std::string_view& key, const std::string_view& value);

  status get(const std::string_view& key, std::string* value);

 private:
  std::unique_ptr<rocksdb::DB> db_;
};

}  // namespace raft

}  // namespace fver

#endif