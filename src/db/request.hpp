#ifndef SRC_DB_REQUEST_H_
#define SRC_DB_REQUEST_H_

#include <cstring>
#include <string>
#include "src/db/status.hpp"

namespace fver {

namespace db {

struct PutContext {
  std::string_view key;
  std::string_view value;
  Status code;
};

struct GetContext {
  std::string_view key;
  std::string_view value;
  Status code;
};

struct DeleteContext {
  std::string_view key;
  Status code;
};


}  // namespace db

}  // namespace fver

#endif