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
  SetContext(const std::string_view& key_view,
             const std::string_view& value_view)
      : key(key_view), value(value_view) {}
};

struct PutContext {
  std::string key;
  std::string value;
  Status code;
};

struct GetContext {
  std::string key;
  std::string value;
  Status code;
  GetContext(const std::string_view& key_view) : key(key_view) {}
};

struct DeleteContext {
  std::string key;
  Status code;
};

struct ExistContext {
  std::string key;
  bool is_exist;
  Status code;
};

}  // namespace db

}  // namespace fver

#endif