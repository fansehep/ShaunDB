#ifndef SRC_DB_REQUEST_H_
#define SRC_DB_REQUEST_H_

#include <cstring>
#include <string>

#include "src/db/status.hpp"

namespace fver {

namespace db {

struct SetContext {
  std::string_view key;
  std::string_view value;
  Status code;
  SetContext(const std::string_view& key_view,
             const std::string_view value_view)
      : key(key_view), value(value_view) {}
};

struct PutContext {
  std::string_view key;
  std::string_view value;
  Status code;
};

struct GetContext {
  std::string_view key;
  std::string_view value;
  Status code;
  GetContext(const std::string_view& key_view) : key(key_view) {}
};

struct DeleteContext {
  std::string_view key;
  Status code;
};

}  // namespace db

}  // namespace fver

#endif