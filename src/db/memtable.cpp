#include "src/db/memtable.hpp"

#include "src/base/log/logging.hpp"
#include "src/db/comp.hpp"
#include "src/db/key_format.hpp"
#include "src/db/request.hpp"
#include "src/db/status.hpp"
#include "src/db/wal_writer.hpp"

namespace fver {

namespace db {

// reference 引用计数
Memtable::Memtable() : isReadonly_(false), refs_(0) {}

// clang-format off

/*
  | key_size      | key_value | sequence_number | value_type | value_size | value_val |
  | varint byte   |  ? byte   |    8 byte       |   1 byte   |   4 byte   | ? byte    |
// 1 <= varient_size <= 5
*/

// clang-format on

static thread_local std::string simple_set_str;

void Memtable::Set(const std::shared_ptr<SetContext>& set_context) {
  auto key_size_32_varint_size = varintLength(set_context->key.size());
  auto value_size_32_varint_size = varintLength(set_context->value.size());
  auto sequence_number_64_varint_size = varintLength(set_context->value.size());
  simple_set_str = fmt::format(
      "{}{}{}{}{}{}", format32_vec[key_size_32_varint_size], set_context->key,
      format64_vec[sequence_number_64_varint_size], kEmpty1Space,
      format32_vec[value_size_32_varint_size], set_context->value);
  // 赋值 key_size;
  // formatEncodeFixed32(set_context->key.size(), simple_set_str.data());
  char* start_ptr = simple_set_str.data();
  start_ptr = encodeVarint32(start_ptr, set_context->key.size());
  //
  // 赋值 sequence_number
  // formatEncodeFixed64(set_context->key.size(),
  //                     simple_set_str.data() + 4 + set_context->key.size());
  //
  start_ptr += set_context->key.size();
  start_ptr = encodeVarint64(start_ptr, set_context->number);
  //
  // 赋值 value_type, 表示key 还活着,
  // formatEncodeFixed8(ValueType::kTypeValue,
  //                    simple_set_str.data() + 4 + set_context->key.size() +
  //                    8);
  //
  formatEncodeFixed8(ValueType::kTypeValue, start_ptr);
  start_ptr += 1;
  //
  // 赋值 value.size(),
  // formatEncodeFixed32(set_context->value.size(),
  //                     simple_set_str.data() + 4 + set_context->key.size() +
  //                     9);
  //
  encodeVarint32(start_ptr, set_context->value.size());

  // 插入到 bloomFilter, 方便快速进行判断
  bloomFilter_.Insert(set_context->key);
  // 如果 key 已经存在, 那么根据自定义比较器应该会覆盖
  auto origin_memMap_size = memMap_.size();
  auto origin_str_size = simple_set_str.size();
  memMap_.insert(std::move(simple_set_str));
  // set 请求有可能覆盖原有的值
  // 只有当 memMap_ 的值发生改变, 才会去增加 memMap 的size
  if (memMap_.size() > origin_memMap_size) {
    memSize_ += origin_str_size;
  }
}

static thread_local std::string simple_get_str;

void Memtable::Get(const std::shared_ptr<GetContext>& get_context) {
  // 使用 bloomFilter 进行快速判断
  if (false == bloomFilter_.IsMatch(get_context->key)) {
    get_context->code.setCode(StatusCode::kNotFound);
    LOG_INFO("bloomfilter key: {} can not found", get_context->key);
    return;
  }

  auto pre_key_var_size = varintLength(get_context->key.size());
  auto pre_number_var_size = varintLength(get_context->number);

  simple_get_str =
      fmt::format("{}{}{}", format32_vec[pre_key_var_size], get_context->key,
                  format64_vec[pre_number_var_size]);
  // 赋值 key_size
  char* start_ptr = simple_get_str.data();
  start_ptr = encodeVarint32(start_ptr, get_context->key.size());
  // 赋值 number
  start_ptr += get_context->key.size();

  start_ptr = encodeVarint64(start_ptr, get_context->number);

  auto iter = memMap_.find(simple_get_str);

  if (iter == memMap_.end()) {
    LOG_WARN("key: {} can not find in memTable", get_context->key);
    get_context->code.setCode(StatusCode::kNotFound);
    return;
  }
  // 获取 key_size;
  uint32_t mem_key_size = 0;
  auto end_ptr = getVarint32Ptr(iter->data(), iter->data() + 5, &mem_key_size);
  assert(end_ptr != nullptr);

  uint64_t iter_number;
  end_ptr = getVarint64Ptr(end_ptr + mem_key_size, end_ptr + mem_key_size + 9,
                           &iter_number);

  // 获取 key, value 当前状态
  uint8_t key_type = formatDecodeFixed8(end_ptr);
  // key_type 处于已经被删除了, 直接返回即可
  if (key_type == ValueType::kTypeDeletion) {
    get_context->code.setCode(StatusCode::kNotFound);
    LOG_WARN("get key: {} has been delete", get_context->key);
    return;
  }

  // 获取 value_size;
  uint32_t mem_value_size = 0;
  end_ptr = getVarint32Ptr(end_ptr + 1, end_ptr + 6, &mem_value_size);

  //

  // 获取 value_view
  std::string_view value_view(end_ptr, mem_value_size);
  // 给 get_context 赋值
  get_context->value = value_view;
  get_context->code.setCode(StatusCode::kOk);
  LOG_INFO("get key: {} value: {} ok", get_context->key, value_view);
}

static thread_local std::string simple_del_str;

void Memtable::Delete(const std::shared_ptr<DeleteContext>& del_context) {
  // 使用 bloomFilter 进行快速判断
  if (false == bloomFilter_.IsMatch(del_context->key)) {
    del_context->code.setCode(StatusCode::kNotFound);
    LOG_INFO("from bloomfilter delete key: {} can not found", del_context->key);
    return;
  }
  auto key_size_32_varint_size = varintLength(del_context->key.size());
  auto sequence_number_64_varint_size = varintLength(del_context->number);

  simple_del_str = fmt::format(
      "{}{}{}{}", format32_vec[key_size_32_varint_size], del_context->key,
      format64_vec[sequence_number_64_varint_size], kEmpty1Space);
  char* start_ptr = simple_set_str.data();
  start_ptr = encodeVarint32(start_ptr, del_context->key.size());

  auto iter = memMap_.find(simple_get_str);

  if (iter == memMap_.end()) {
    LOG_WARN("del key: {} can not find in memtable", del_context->key);
    del_context->code.setCode(StatusCode::kNotFound);
    return;
  }
  //
  // 标记 删除
  // 获取 key_size;
  uint32_t mem_key_size = 0;
  auto end_ptr = getVarint32Ptr(iter->data(), iter->data() + 5, &mem_key_size);
  // 标志位删除.
  uint64_t mem_number = 0;
  end_ptr = getVarint64Ptr(end_ptr + mem_key_size, end_ptr + 9, &mem_number);
  //
  *(const_cast<char*>(end_ptr)) = ValueType::kTypeDeletion;
  LOG_INFO("del key: {} ok", del_context->key);
}

uint32_t Memtable::getMemSize() { return memSize_; }

void Memtable::setReadOnly() { isReadonly_ = true; }

void Memtable::setNumber(const uint32_t number) { memtable_number_ = number; }

uint32_t Memtable::getMemNumber() const { return memtable_number_; }

void Memtable::addRefs() { refs_++; }

uint32_t Memtable::getRefs() { return refs_.load(); }

void Memtable::decreaseRefs() { refs_--; }

auto Memtable::getMemTableRef() -> MemBTree& { return memMap_; }

}  // namespace db

}  // namespace fver