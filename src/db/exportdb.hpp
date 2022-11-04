#ifndef SRC_DB_EXPORT_DB_H_
#define SRC_DB_EXPORT_DB_H_

#include <memory>

#include "src/base/noncopyable.hpp"
#include "src/db/request.hpp"
#include "src/db/shared_memtable.hpp"
#include "src/db/wal_writer.hpp"
#include "src/util/file/wal_log.hpp"

using ::fver::base::NonCopyable;
using ::fver::db::SharedMemtable;
using ::fver::util::file::WalLog;

namespace fver {
namespace db {

/*
WAL_LOG 的文件名称是  WAL_LOG.log
--- db_path
---- ShaunDB-20xx-xx-xx.log : 普通日志
---- ShaunDB-WAL_LOG.log : 预写日志,
---- sstable_0.stb -|
---- sstable_1.stb  |
---- sstable_2.stb  |
---- .              | --> SStable 文件, 默认最高 7 层.
---- .              |
---- .              |
----              --
*/

/*
  Minor_Compaction => 将内存数据库 Immutable 中的所有的数据写到 SStable中去
  Major_Compaction => 合并新的 SStable
*/

const std::string kDefaultWalLogFileName = "WAL_LOG.log";

struct DBConfig {
  // 预写日志的容量大小
  uint32_t wal_log_size;
  // kv 引擎文件所放置的目录
  std::string db_path;
  // 触发 compaction 的阈值 / byte
  uint32_t compaction_trigger;
};

//
// TODO: 我们应该考虑 const std::shared_ptr<>&
// 与 std::shared_ptr<> 的生命周期问题
// const std::shared_ptr<>& 并不会增加引用计数
// 此处做参数传递可以尽量使用 const std::shared_ptr<>&
//

struct WALSequenceNumber {
  std::atomic<uint64_t> number_{0};
  void operator++() { number_++; }
  uint64_t getValue() { return number_.load(); }
};

class DB : public NonCopyable {
 public:
  DB() = default;
  ~DB() = default;
  bool Init(const DBConfig& config);
  void Set(const std::shared_ptr<SetContext>& set_context);
  void Get(const std::shared_ptr<GetContext>& get_context);
  void Delete(const std::shared_ptr<DeleteContext>& del_context);

 private:
  // 预写日志
  std::unique_ptr<WalWriter> walLog_;
  // 内存表

  // 预写日志的ID
  // TODO: 事实上, 应该可以不需要使用 global_number 一个可以直接写入
  WALSequenceNumber global_number_;

  // 简单的序列号
  std::atomic<uint64_t> request_number_;

  // 当传入的目录是一个空目录时, 可以重新创建一个新的 path.
  void NewDBimp();

  // 判断传入 路径是否为空
  bool isEmptyDir(const std::string& db_path);
};

}  // namespace db

}  // namespace fver

#endif