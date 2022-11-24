#include "src/db/exportdb.hpp"

#include "src/base/log/logging.hpp"
#include "src/db/compactor.hpp"
#include "src/db/memtable.hpp"
#include "src/db/memtable_view_manager.hpp"
#include "src/db/request.hpp"
#include "src/db/shared_memtable.hpp"
#include "src/db/status.hpp"
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
    //  NewDBimp(config);
    // return true;
  } else {
    // 从现有的预写日志中读取数据
  }
  NewDBimp(config);
  return true;
}

void DB::NewDBimp(const DBConfig& db_config) {
  // 预写日志初始化
  Status wal_log_init_status;
  shared_memtable_ = std::make_shared<SharedMemtable>();
  walLog_->Init(db_config.db_path, "ShaunDB_WAL.log", &wal_log_init_status);
  if (wal_log_init_status.getCode() != StatusCode::kOk) {
    LOG_ERROR("error wal_log can not init");
  }
  // 初始化内存表
  // 初始化压缩者
  comp_actor_ = std::make_shared<Compactor>();
  
  // memtable 视图初始化
  memviewtable_manager_ = std::make_shared<MemTableViewManager>();
  //
  memviewtable_manager_->Init(db_config.memtable_N);
  LOG_INFO("compactor make_shared success");
  comp_actor_->Init(db_config.memtable_N, db_config.compactor_thread_size,
                    db_config.db_path);
  // compworker 拥有 Compactor 共享权
  comp_actor_->SetCompWorkerCompactorRef(comp_actor_); 
  LOG_INFO("compactor init success");
  // 初始化 shared_memtable
  shared_memtable_->Init(db_config);
  // 让 shared_memtable 同时获得 Compactor 的引用
  shared_memtable_->SetCompactorRef(comp_actor_);
  comp_actor_->SetSharedMemTableRef(shared_memtable_);
  // comp_actor 让 comp_worker 获取引用
  comp_actor_->SetSharedMemTableViewSSRef(memviewtable_manager_);
  // shared_memtable 同时持有 memtable_view_vec 的所有权
  shared_memtable_->SetMemTableViewRef(memviewtable_manager_);
  shared_memtable_->Run();
  comp_actor_->Run();
  LOG_INFO("init success");
}

void DB::Delete(const std::shared_ptr<DeleteContext>& del_context) {
  std::string simple_log;
  DeleteContextWalLogFormat(del_context, global_number_.getValue(),
                            &simple_log);
  ++global_number_;
  walLog_->AddRecord(simple_log);
  shared_memtable_->Delete(del_context);
}

void DB::Get(const std::shared_ptr<GetContext>& get_context) {
  shared_memtable_->Get(get_context);
}

void DB::Set(const std::shared_ptr<SetContext>& set_context) {
  std::string simple_log;
  SetContextWalLogFormat(set_context, global_number_.getValue(), &simple_log);
  ++global_number_;
  walLog_->AddRecord(simple_log);
  shared_memtable_->Set(set_context);
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