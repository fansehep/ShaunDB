#include "src/db/exportdb.hpp"

#include "src/base/log/logging.hpp"
#include "src/db/compactor.hpp"
#include "src/db/request.hpp"
#include "src/db/shared_memtable.hpp"
#include "src/db/wal_writer.hpp"

extern "C" {
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
}

namespace fver {

namespace db {

bool DB::Init(const DBConfig& config) {
  walLog_ = std::make_unique<WalWriter>();
  // 初始化一个新的 DB
  if (true == isEmptyDir(config.db_path)) {
    NewDBimp(config);
    return true;
  } else {
    // 从现有的预写日志中读取数据
  }
}

void DB::NewDBimp(const DBConfig& db_config) {
  // 初始化内存表
  // 初始化压缩者
  comp_actor_ = std::make_shared<Compactor>();
  //
  comp_actor_->Init(db_config.memtable_N, db_config.compactor_thread_size,
                    db_config.db_path);
  // 让 shared_memtable 同时获得 Compactor 的引用
  shared_memtable_.SetCompactorRef(comp_actor_);
  // 初始化 shared_memtable
  shared_memtable_.Init(db_config.memtable_N, db_config.memtable_trigger_size);
}

void DB::Delete(const std::shared_ptr<DeleteContext>& del_context) {
  static thread_local std::string simple_log;
  DeleteContextWalLogFormat(del_context, global_number_.getValue(),
                            &simple_log);
  ++global_number_;
  walLog_->AddRecord(simple_log);
}

void DB::Get(const std::shared_ptr<GetContext>& get_context) {}

void DB::Set(const std::shared_ptr<SetContext>& set_context) {
  static thread_local std::string simple_log;
  SetContextWalLogFormat(set_context, global_number_.getValue(), &simple_log);
  ++global_number_;
  walLog_->AddRecord(simple_log);
}

bool DB::isEmptyDir(const std::string& db_path) {
  auto isOk = ::access(db_path.c_str(), F_OK);
  // 该目录下没有 其他文件, 初始化一个新的 db
  if (-1 == isOk) {
    LOG_TRACE("init a new empty db", db_path);
    return true;
  }
  return false;
}

}  // namespace db

}  // namespace fver