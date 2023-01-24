#include "src/db/key_format.hpp"

#include "fmt/format.h"

#include <cstdint>
#include <string_view>

#include "src/base/log/logging.hpp"
#include "src/db/wal_writer.hpp"
#include "src/util/crc32.hpp"

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

uint8_t formatDecodeFixed8(const char* data) { return data[0]; }

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

std::string_view getMemTableViewKeyView(std::string_view& iter) {
  uint32_t key_size;
  auto end_ptr = getVarint32Ptr(iter.data(), iter.data() + 5, &key_size);
  assert(end_ptr != nullptr);
  return std::string_view(end_ptr, key_size);
}

std::string_view getMemTableViewKeyViewIter(MemBTreeView::iterator iter) {
  uint32_t key_size;
  auto end_ptr = getVarint32Ptr(iter->data(), iter->data() + 5, &key_size);
  assert(end_ptr != nullptr);
  return std::string_view(end_ptr, key_size);
}

//
SSTableKeyValueStyle formatMemTableToSSTable(const MemBTree::iterator& iter) {
  SSTableKeyValueStyle sstable_key_value;
  uint32_t key_size;
  uint32_t value_size;
  auto key_end_ptr = getVarint32Ptr(iter->data(), iter->data() + 5, &key_size);
  sstable_key_value.key_view = std::string_view(key_end_ptr, key_size);
  uint64_t number_value;
  auto next_value_end_ptr = getVarint64Ptr(
      key_end_ptr + key_size, key_end_ptr + key_size + 9, &number_value);
  sstable_key_value.isExist = formatDecodeFixed8(next_value_end_ptr);
  key_end_ptr = getVarint32Ptr(next_value_end_ptr + 1, next_value_end_ptr + 6,
                               &value_size);
  sstable_key_value.value_view = std::string_view(key_end_ptr, value_size);
  return sstable_key_value;
}

SSTableKeyValueStyle formatMemTableToSSTableStr(std::string& str) {
  SSTableKeyValueStyle sstable_key_value;
  uint32_t key_size;
  uint32_t value_size;
  auto key_end_ptr = getVarint32Ptr(str.data(), str.data() + 5, &key_size);
  sstable_key_value.key_view = std::string_view(key_end_ptr, key_size);
  // 可能这里只插入了一条被删除的记录.
  // 那么插入删除的数据格式是 | key_size | key_val |
  // 所以就没有 value
  // 这里需要特判
  if ((sstable_key_value.key_view.data() + sstable_key_value.key_view.size()) ==
      (str.data() + str.size())) {
    sstable_key_value.isExist = false;
    return sstable_key_value;
  }
  //
  uint64_t number_value;
  auto next_value_end_ptr = getVarint64Ptr(
      key_end_ptr + key_size, key_end_ptr + key_size + 9, &number_value);
  //
  sstable_key_value.isExist = formatDecodeFixed8(next_value_end_ptr);
  key_end_ptr = getVarint32Ptr(next_value_end_ptr + 1, next_value_end_ptr + 6,
                               &value_size);
  sstable_key_value.value_view = std::string_view(key_end_ptr, value_size);
  return sstable_key_value;
}

// 每隔 16 个进行前缀压缩.
Format16PrefixResult Format16PrefixStr(
    const std::vector<SSTableKeyValueStyle>& sstable_vec) {
  //
  const int sstable_vec_size = sstable_vec.size();
  // 当 sstable_vec 只有 0 个 或者 1 个时需要特殊处理.
  if (0 == sstable_vec_size || 1 == sstable_vec_size) {
  }
  //
  Format16PrefixResult result;

  //
  int i = 0;
  // 前缀压缩 key 最小的 size
  int prefix_key_min_size = sstable_vec[0].key_view.size();
  for (auto& iter : sstable_vec) {
    prefix_key_min_size =
        std::min<int>(iter.key_view.size(), prefix_key_min_size);
  }

  const int lnner_same_size = sstable_vec_size - 1;

  /*
   * sstable_vec[0] abcdddd
   * sstable_vec[1] abcfff
   * sstable_vec[2] abcll
   * sstable_vec[3] abcz
   * 对这些前缀进行压缩.
   */

  for (; i < prefix_key_min_size; i++) {
    int j = 0;
    //
    for (; j < lnner_same_size; j++) {
      if (sstable_vec[j].key_view[i] != sstable_vec[j + 1].key_view[i]) {
        break;
      }
    }
    // 说明 j 没有 到达 lnner_same_size
    // but i think here goto is right.
    // but no problem.
    if (j != lnner_same_size) {
      break;
    }
  }

  i -= 1;
  if (i == -1) {
    i = 0;
  } else {
    // 相同 key 的前缀.
    result.key_view = std::string_view(sstable_vec[0].key_view.data(), i + 1);
  }
  // 前缀压缩 value 最小的 size
  int prefix_value_min_size = sstable_vec[0].value_view.size();
  for (auto& iter : sstable_vec) {
    prefix_value_min_size =
        std::min<int>(iter.value_view.size(), prefix_value_min_size);
  }
  i = 0;
  for (; i < prefix_value_min_size; i++) {
    int j = 0;
    for (; j < lnner_same_size; j++) {
      if (sstable_vec[j].value_view[i] != sstable_vec[j + 1].value_view[i]) {
        break;
      }
    }

    if (j != lnner_same_size) {
      break;
    }
  }

  i -= 1;
  if (i == -1) {
    i = 0;
  }
  // 最大相同 value 前缀.
  else {
    result.value_view =
        std::string_view(sstable_vec[0].value_view.data(), i + 1);
  }
  // data_kv_赋值.

  return result;
}

char* encodeVarint32(char* dst, const uint32_t v) {
  uint8_t* ptr = reinterpret_cast<uint8_t*>(dst);
  static constexpr int B = 128;
  if (v < (1 << 7)) {
    *(ptr++) = v;

  } else if (v < (1 << 14)) {
    // v | B,
    // 例如: value(二进制表示) =  00000000 00000000 00000000 00000000
    // 通过  value | 128 则可以得到                |--------|
    // 这一部分的值, 然后在进行保存
    // 下面的行为类似
    *(ptr++) = v | B;
    *(ptr++) = v >> 7;
  } else if (v < (1 << 21)) {
    *(ptr++) = v | B;
    *(ptr++) = (v >> 7) | B;
    *(ptr++) = v >> 14;
  } else if (v < (1 << 28)) {
    *(ptr++) = v | B;
    *(ptr++) = (v >> 7) | B;
    *(ptr++) = (v >> 14) | B;
    *(ptr++) = v >> 21;
  } else {
    *(ptr++) = v | B;
    *(ptr++) = (v >> 7) | B;
    *(ptr++) = (v >> 14) | B;
    *(ptr++) = (v >> 21) | B;
    *(ptr++) = v >> 28;
  }
  return reinterpret_cast<char*>(ptr);
}

char* encodeVarint64(char* dst, uint64_t v) {
  static const int B = 128;
  uint8_t* ptr = reinterpret_cast<uint8_t*>(dst);
  // i think 循环是好的
  while (v >= B) {
    *(ptr++) = v | B;
    v >>= 7;
  }
  *(ptr++) = static_cast<uint8_t>(v);
  return reinterpret_cast<char*>(ptr);
}

const char* getVarint32Ptr(const char* p, const char* limit, uint32_t* value) {
  if (p < limit) {
    // char buf[4] = 00000000 00000000 00000000 00000000
    //               |       |        |        |        |
    // 依次取出每个buf[i] 中的值
    uint32_t result = *(reinterpret_cast<const uint8_t*>(p));
    // result & 128(1 << 7), 在判断第一个字节是否为 0
    // 在上面的EncodeVarInt32中, 使用每个字节的第一位来判断后续是否还有数字
    // 如果第一位 == 0, 则表示解码成功, 直接返回即可
    if ((result & 128) == 0) {
      *value = result;
      return p + 1;
    }
  }
  return getVarint32PtrFallback(p, limit, value);
}

const char* getVarint32PtrFallback(const char* p, const char* limit,
                                   uint32_t* value) {
  uint32_t result = 0;
  for (uint32_t shift = 0; shift <= 28 && p < limit; shift += 7) {
    // | 00000000 | 00000000 | 00000000 | 00000000 |
    //
    uint32_t byte = *(reinterpret_cast<const uint8_t*>(p));
    p++;
    // 如果 byte | 00000000 | & (1 << 7) => 第一个字节为 1
    // 则表示后续还有更多的字符需要格式化
    if (byte & 128) {
      // More bytes are present
      result |= ((byte & 127) << shift);
    } else {
      // 后续没有了
      result |= (byte << shift);
      *value = result;
      return reinterpret_cast<const char*>(p);
    }
  }
  return nullptr;
}

const char* getVarint64Ptr(const char* p, const char* limit, uint64_t* value) {
  uint64_t result = 0;
  for (uint32_t shift = 0; shift <= 63 && p < limit; shift += 7) {
    uint64_t byte = *(reinterpret_cast<const uint8_t*>(p));
    p++;
    if (byte & 128) {
      // More bytes are present
      result |= ((byte & 127) << shift);
    } else {
      result |= (byte << shift);
      *value = result;
      return reinterpret_cast<const char*>(p);
    }
  }
  return nullptr;
}

int varintLength(uint64_t v) {
  int len = 1;
  while (v >= 128) {
    v >>= 7;
    len++;
  }
  return len;
}

bool decodeVarint32(std::string* input, uint32_t* value) {
  const char* p = input->data();
  const char* limit = p + input->size();
  const char* q = getVarint32Ptr(p, limit, value);
  if (q == nullptr) {
    return false;
  } else {
    *input = std::string_view(q, limit - q);
    return true;
  }
}

bool decodeVarint64(std::string* input, uint64_t* value) {
  const char* p = input->data();
  const char* limit = p + input->size();
  const char* q = getVarint64Ptr(p, limit, value);
  if (q == nullptr) {
    return false;
  } else {
    *input = std::string_view(q, limit - q);
    return true;
  }
}

}  // namespace db

}  // namespace fver