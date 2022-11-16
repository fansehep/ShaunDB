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

struct DBConfig {

  // 预写日志的容量大小
  uint32_t wal_log_size;

  // kv 引擎文件所放置的目录
  std::string db_path;

  // NightSheep DB 拥有多个内存表
  // 此配置表示 NightSheepDB 的内存表的数量
  uint32_t memtable_N;

  // 单个内存表的最大容量
  // 注意, 一个内存表被写满之后, 并不能立即被刷入
  // 他将会等待 Compactor 被刷入到  SStable 中去
  // 所以峰值内存占用的计算公式是:
  // (2 * memtable_N * memtable_trigger_size) + (2 * memtable_N * 8MB)
  // 注意峰值内存占用, 不然直接 oom
  uint32_t memtable_trigger_size;

  // compactor 的线程个数
  // 当内存表过多时, 映射的 Compactor 如果线程数量太少, Compaction 操作明显变慢
  // BEST: 使用 memtable_trigger_size / 4 个线程 来进行 Compaction
  // 当然, 这里我们还应该考虑单个 memtable 的大小.
  uint32_t compactor_thread_size;

  // 如果打开的目录已经存在, 是否尝试恢复
  // 否: 清空所有数据, 新建立一个 DB
  // 是: 尝试恢复数据, 读取原有的预写日志, 并尝试恢复
  // 尝试恢复之后, 若数据恢复失败, 则会停止启动服务器
  bool isRecover;

};

//
// TODO: 我们应该考虑 const std::shared_ptr<>&
// 与 std::shared_ptr<> 的引用计数问题
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

  SharedMemtable shared_memtable_;

  std::shared_ptr<Compactor> comp_actor_;
};

}  // namespace db

}  // namespace fver

#endif