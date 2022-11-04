#ifndef SRC_DB_COMPARATOR_H_
#define SRC_DB_COMPARATOR_H_

#include <string>

#include "src/base/noncopyable.hpp"
#include "src/db/key_format.hpp"

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
                  std::string_view const& ref) const {
    // uint32_t lef_key_size = formatDecodeFixed32(lef.data());
    // uint32_t ref_key_size = formatDecodeFixed32(ref.data());
    // std::string_view key_view(lef.data() + 4, lef_key_size);
    // std::string_view value_view(ref.data() + 4, ref_key_size);
    // return key_view > value_view;
    uint32_t lef_key_size = static_cast<uint32_t>(lef[0]) |
                            static_cast<uint32_t>(lef[1]) << 8 |
                            static_cast<uint32_t>(lef[2]) << 16 |
                            static_cast<uint32_t>(lef[3]) << 24;
    uint32_t ref_key_size = static_cast<uint32_t>(ref[0]) |
                            static_cast<uint32_t>(ref[1]) << 8 |
                            static_cast<uint32_t>(ref[2]) << 16 |
                            static_cast<uint32_t>(ref[3]) << 24;
    std::string_view lef_key_view(lef.data() + 4, lef_key_size);
    std::string_view ref_key_view(ref.data() + 4, ref_key_size);
    return lef_key_view < ref_key_view;
  }
};

}  // namespace db

}  // namespace fver

#endif