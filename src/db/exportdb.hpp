#ifndef SRC_DB_EXPORT_DB_H_
#define SRC_DB_EXPORT_DB_H_

#include <memory>

#include "src/base/noncopyable.hpp"
#include "src/db/request.hpp"
#include "src/util/file/wal_log.hpp"
#include "src/db/shared_memtable.hpp"

using ::fver::base::NonCopyable;
using ::fver::util::file::WalLog;
using ::fver::db::SharedMemtable;

namespace fver {
namespace db {

class DB : public NonCopyable {
 public:
  DB();
  void Init();
  bool Put(std::shared_ptr<PutContext> put_context);
  bool Get(std::shared_ptr<GetContext> get_context);

 private:
  // 预写日志
  std::unique_ptr<WalLog> walLog_;
  // 内存共享表
  SharedMemtable shared_memtable_;
};

}  // namespace db

}  // namespace fver

#endif