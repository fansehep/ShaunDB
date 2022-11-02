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


struct DBConfig {
  // 预写日志的容量大小
  uint32_t wal_log_size;
};

//
//TODO: 我们应该考虑 const std::shared_ptr<>&
// 与 std::shared_ptr<> 的生命周期问题
// const std::shared_ptr<>& 并不会增加引用计数
// 此处做参数传递可以尽量使用 const std::shared_ptr<>&
//

struct SequenceNumber {
  uint64_t number_;
};


class DB : public NonCopyable {
 public:
  DB() = default;
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