#ifndef SRC_DB_REQUEST_H_
#define SRC_DB_REQUEST_H_

#include <cstring>
#include <string>

#include "src/db/status.hpp"

namespace fver {

namespace db {

enum Request : uint32_t {
  kSet_Request,
  kPut_Request,
  kGet_Request,
  kDel_Request,
  KExi_Request,
};

// 这里采取string 复制的方式, 有些许性能开销
struct SetContext {
  SetContext() = default;
  std::string key;
  std::string value;
  Status code;
  uint64_t number;
  SetContext(const std::string_view& key_view,
             const std::string_view& value_view)
      : key(key_view), value(value_view) {}
};

struct PutContext {
  PutContext() = default;
  std::string key;
  std::string value;
  Status code;
  uint64_t number;
};

//TODO: get request need a condition_variable to notify the wait thread
// to tell the 
struct GetContext {
  GetContext() = default;
  std::string key;
  std::string value;
  Status code;
  uint64_t number;
  GetContext(const std::string_view& key_view) : key(key_view) {}
};

struct DeleteContext {
  DeleteContext() = default;
  std::string key;
  Status code;
  uint64_t number;
};

struct ExistContext {
  ExistContext() = default;
  std::string key;
  bool is_exist;
  Status code;
};

}  // namespace db

}  // namespace fver

#endif