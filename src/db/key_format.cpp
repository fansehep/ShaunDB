#include "src/db/key_format.hpp"

#include "src/util/crc32.hpp"
#include <fmt/format.h>
#include "src/db/wal_writer.hpp"
#include "src/base/log/logging.hpp"

extern "C" {
#include <assert.h>
}

namespace fver {

namespace db {

// 从 data[0 - 4] 中解码出 uint32_t
uint32_t formatDecodeFixed32(char* data) {
  uint8_t* buf = reinterpret_cast<uint8_t*>(data);
  return (static_cast<uint32_t>(buf[0])) | static_cast<uint32_t>(buf[1]) << 8 |
         static_cast<uint32_t>(buf[2]) << 16 |
         static_cast<uint32_t>(buf[3]) << 24;
}

// 从 data[0 - 8] 中解码 出 uint64_t
uint64_t formatDecodeFixed64(char* data) {
  const uint8_t* const buffer = reinterpret_cast<const uint8_t*>(data);

  // Recent clang and gcc optimize this to a single mov / ldr instruction.
  return (static_cast<uint64_t>(buffer[0])) |
         (static_cast<uint64_t>(buffer[1]) << 8) |
         (static_cast<uint64_t>(buffer[2]) << 16) |
         (static_cast<uint64_t>(buffer[3]) << 24) |
         (static_cast<uint64_t>(buffer[4]) << 32) |
         (static_cast<uint64_t>(buffer[5]) << 40) |
         (static_cast<uint64_t>(buffer[6]) << 48) |
         (static_cast<uint64_t>(buffer[7]) << 56);
}

void formatEncodeFixed8(const uint8_t value, char* data) { data[0] = value; }

uint8_t formatDecodeFixed8(char* data) { return data[0]; }

// 将 uint32_t value 编码到 data 中去
void formatEncodeFixed32(const uint32_t value, char* data) {
  data[0] = static_cast<uint8_t>(value);
  data[1] = static_cast<uint8_t>(value >> 8);
  data[2] = static_cast<uint8_t>(value >> 16);
  data[3] = static_cast<uint8_t>(value >> 24);
}

// 将 uint64_t value 编码到 data 中去
void formatEncodeFixed64(const uint64_t value, char* data) {
  uint8_t* const buf = reinterpret_cast<uint8_t*>(data);
  buf[0] = static_cast<uint8_t>(value);
  buf[1] = static_cast<uint8_t>(value >> 8);
  buf[2] = static_cast<uint8_t>(value >> 16);
  buf[3] = static_cast<uint8_t>(value >> 24);
  buf[4] = static_cast<uint8_t>(value >> 32);
  buf[5] = static_cast<uint8_t>(value >> 40);
  buf[6] = static_cast<uint8_t>(value >> 48);
  buf[7] = static_cast<uint8_t>(value >> 56);
}

// clang-format off

// crc32_check_sum 用于校验数据完整度
// | crc32_check_sum | sequence_number |   type    | key_length |  value_length | key_val   | value_val | kFlagLogEnd          |  
// |   4 byte        |   8 byte        |    1 byte | 4 byte     |    4 byte     |  ? byte   |   ? byte  |   sizeof(kFlagLogEnd)|

// clang-format on

void SetContextWalLogFormat(const std::shared_ptr<SetContext>& set_context,
                            const uint64_t number, std::string* log) {
  assert(nullptr != log);
  *log = fmt::format("{}{}{}{}{}{}{}{}", kEmpty4Space, kEmpty8Space, " ",
                     kEmpty4Space, kEmpty4Space, set_context->key,
                     set_context->value, kFlagLogEnd);
  // 计算 crc32 的值
  auto crc32_check_sum = util::Value(log->c_str() + 4, log->size() - 4);
  LOG_DEBUG("crc32_check_sum = {}", crc32_check_sum);
  // 赋值 crc32_check_sum
  formatEncodeFixed32(crc32_check_sum, log->data());
  // 赋值 number
  formatEncodeFixed64(number, log->data() + 4);
  // 赋值 type
  formatEncodeFixed8(kSet_Request, log->data() + 12);
  // 赋值 key_length
  formatEncodeFixed32(set_context->key.size(), log->data() + 13);
  // 赋值 value_length
  formatEncodeFixed32(set_context->value.size(), log->data() + 17);
  // 结束
  return;
}

void PutContextWalLogFormat(const std::shared_ptr<PutContext>& put_context,
                            const uint64_t number, std::string* log) {
  assert(nullptr != log);
  *log = fmt::format("    {:0>8}{:0>1}{:>4}{:>4}{}{}{}", number,
                     Request::kPut_Request, put_context->key.size(),
                     put_context->value.size(), put_context->key,
                     put_context->value, kFlagLogEnd);
  auto crc32_check_sum = util::Value(log->c_str() + 4, log->size() - 4);
  formatEncodeFixed32(crc32_check_sum, log->data());
  return;
}

// | crc32_check_sum | type   | key_length | key_val | kFlagLogEnd |
// |     4 byte      | 1 byte | key_length | key_val | ...         |

void DeleteContextWalLogFormat(
    const std::shared_ptr<DeleteContext>& del_context, const uint64_t number,
    std::string* log) {
  assert(nullptr != log);
  *log =
      fmt::format("    {:0>8}{:0>1}{:>4}{:>4}{}", number, Request::kDel_Request,
                  del_context->key.size(), del_context->key, kFlagLogEnd);
  auto crc32_check_sum = util::Value(log->c_str() + 4, log->size() - 4);
  formatEncodeFixed32(crc32_check_sum, log->data());
}

//
inline SSTableKeyValueStyle formatMemTableToSSTable(const MemBTree::iterator &iter) {
  SSTableKeyValueStyle sstable_key_value;
  uint32_t key_size = formatDecodeFixed32(iter->data()); 
  uint32_t value_size = formatDecodeFixed32(iter->data() + key_size + 13);
  sstable_key_value.isExist = formatDecodeFixed8(iter->data() + 4 + key_size);
  sstable_key_value.key_view = std::string_view(iter->data() + 4, key_size);
  sstable_key_value.value_view = std::string_view(iter->data() + 4 + key_size + 13, value_size);
  return sstable_key_value;
}


}  // namespace db

}  // namespace fver