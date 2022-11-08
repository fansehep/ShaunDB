#ifndef SRC_DB_COMPARATOR_H_
#define SRC_DB_COMPARATOR_H_

#include <string>

#include "src/base/noncopyable.hpp"

using ::fver::base::NonCopyable;

namespace fver {

namespace db {

// for memtable key comparate
// 对于单个 Memtable 来说,
// 一个 key 的格式
// clang-format off
/*
  | key_size | key_value | sequence_number | value_type | value_size | value_val |
  | 4 byte   |  ? byte   |    8 byte       |   1 byte   |   4 byte   | ? byte    |
*/

// clang-format on

struct Comparator {
  bool operator()(std::string_view const& lef,
                  std::string_view const& ref) const;
};

}  // namespace db

}  // namespace fver

#endif