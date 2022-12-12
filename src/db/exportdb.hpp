#ifndef SRC_DB_EXPORT_DB_H_
#define SRC_DB_EXPORT_DB_H_

#include <memory>

#include "src/base/noncopyable.hpp"
#include "src/db/memtable_view.hpp"
#include "src/db/request.hpp"
#include "src/db/shared_memtable.hpp"
#include "src/db/wal_writer.hpp"
#include "src/db/dbconfig.hpp"
#include "src/util/file/wal_log.hpp"
#include "src/db/memtable_view_manager.hpp"
#include "src/db/compactor.hpp"
#include "src/db/sstable.hpp"


using ::fver::base::NonCopyable;
using ::fver::db::SharedMemtable;
using ::fver::util::file::WalLog;

// NightSheep engine
// 夜羊db engine
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


//
// TODO: 我们应该考虑 const std::shared_ptr<>&
// 与 std::shared_ptr<> 的引用计数问题
// const std::shared_ptr<>& 并不会增加引用计数
// 此处做参数传递可以尽量使用 const std::shared_ptr<>&
//

extern DBConfig g_dbconfig;

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
  void Snapshot(const std::shared_ptr<SnapShotContext>& snapshot_context);
 private:
  // 预写日志
  std::unique_ptr<WalWriter> walLog_;

  // 预写日志的ID
  // TODO: 事实上, 应该可以不需要使用 global_number 一个可以直接写入
  // 但这里我们使用环形日志, 如果使用 global_number
  // 可以快速判断一个环形日志的头尾
  WALSequenceNumber global_number_;

  // 简单的序列号
  std::atomic<uint64_t> request_number_;

  // 当传入的目录是一个空目录时, 可以重新创建一个新的 path.
  void NewDBimp(const DBConfig& config);

  // 判断传入 路径是否为空
  bool isEmptyDir(const std::string& db_path);

  std::shared_ptr<SharedMemtable> shared_memtable_;

  // 后台做 Compaction 的线程
  std::shared_ptr<Compactor> comp_actor_;

  // 管理 MemTable_View_vew
  // 当 comp_actor 负责将数据压入到磁盘之后
  // 会产生一个 memtable_view 压入到 MemTableViewManager 中.
  std::shared_ptr<MemTableViewManager> memviewtable_manager_;
};

}  // namespace db

}  // namespace fver

#endif