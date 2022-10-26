#ifndef SRC_DB_EXPORT_DB_H_
#define SRC_DB_EXPORT_DB_H_

#include <memory>

#include "src/base/noncopyable.hpp"
#include "src/db/request.hpp"
#include "src/db/shared_memtable.hpp"
#include "src/util/file/wal_log.hpp"

using ::fver::base::NonCopyable;
using ::fver::db::SharedMemtable;
using ::fver::util::file::WalLog;

namespace fver {
namespace db {

class DB : public NonCopyable {
 public:
  DB();
  ~DB() = default;
  void Init();
  void Set(std::shared_ptr<SetContext> set_context);
  void Put(std::shared_ptr<PutContext> put_context);
  void Get(std::shared_ptr<GetContext> get_context);
  void Delete(std::shared_ptr<DeleteContext> del_context);

 private:
  // 预写日志
  std::unique_ptr<WalLog> walLog_;
  // 内存表
  SharedMemtable shared_memtable_;
};

}  // namespace db

}  // namespace fver

#endif