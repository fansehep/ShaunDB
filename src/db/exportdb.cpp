#include "src/db/exportdb.hpp"

#include "src/base/log/logging.hpp"
#include "src/db/request.hpp"
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
    NewDBimp();
    return true;
  } else {
    // 从现有的预写日志中读取数据
  }
}


void DB::NewDBimp() {

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
  DIR* dir = ::opendir(db_path.c_str());
  struct dirent* ent;
  // 该目录下没有 其他文件, 初始化一个新的 db
  if (nullptr == dir) {
    LOG_TRACE("init a new empty db", db_path);
    return true;
  }
  return false;
}

}  // namespace db

}  // namespace fver