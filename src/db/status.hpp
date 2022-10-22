#ifndef SRC_DB_STATUS_H_
#define SRC_DB_STATUS_H_

#include <cstring>
#include <string>

namespace fver {
namespace db {

enum StatusCode {
  // key 未相关
  kNotFound,
  // IO 发生错误
  kIoError,
  // 操作不支持
  kNotSupported,
  // 操作正常
  kOk,
  // 发生异常
  kCorruption,
  // 非法参数
  kInvalidArgument,
};

const std::string statuslev[] = {
    "not found", "io error",   "not supported",
    "ok",        "corruption", "invalidargument",
};

class Status {
 public:
  void setCode(StatusCode code);
  StatusCode getCode();
  std::string_view getCodeStr();
 private:
  StatusCode code_;
  std::string_view code_ptr_;
};

}  // namespace db

}  // namespace fver

#endif