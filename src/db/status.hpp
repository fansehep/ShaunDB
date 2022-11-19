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
  // 被删除
  kDelete,
};

const std::string statuslev[] = {
    "not found", "io error",   "not supported",
    "ok",        "corruption", "invalidargument",
};

class Status {
 public:
  Status();
  void setCode(StatusCode code);
  StatusCode getCode();
  std::string_view getCodeStr();
  void SetErrorLog(const std::string& error_log);
 private:
  // 携带上错误的详细信息
  std::string error_log_;
  // 错误码
  StatusCode code_;
  // 为了日志的打印, 携带上当前状态码所对应的 std::string_view 详细信息
  std::string_view code_ptr_;
};

}  // namespace db

}  // namespace fver

#endif