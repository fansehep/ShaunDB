#include "src/db/memtable.hpp"

#include "src/base/log/logging.hpp"
#include "src/db/key_format.hpp"
#include "src/db/request.hpp"
#include "src/db/status.hpp"
#include "src/db/wal_writer.hpp"

namespace fver {

namespace db {

Memtable::Memtable() : isReadonly_(false), maxSize_(0) {}

// clang-format off

/*
  | key_size | key_value | sequence_number | value_type | value_size | value_val |
  | 4 byte   |  ? byte   |    8 byte       |   1 byte   |   4 byte   | ? byte    |
*/

// clang-format on

static thread_local std::string simple_set_str;

void Memtable::Set(const std::shared_ptr<SetContext> set_context) {
  simple_set_str =
      fmt::format("{}{}{}{}{}{}", kEmpty4Space, set_context->key, kEmpty8Space,
                  kEmpty1Space, kEmpty4Space, set_context->value);
  // 赋值 key_size;
  formatEncodeFixed32(set_context->key.size(), simple_set_str.data());
  // 赋值 sequence_number
  formatEncodeFixed64(set_context->key.size(),
                      simple_set_str.data() + 4 + set_context->key.size());
  // 赋值 value_type, 表示key 还活着,
  formatEncodeFixed8(ValueType::kTypeValue,
                     simple_set_str.data() + 4 + set_context->key.size() + 8);
  // 赋值 value.size(),
  formatEncodeFixed32(set_context->value.size(),
                      simple_set_str.data() + 4 + set_context->key.size() + 9);

  // 使用 读写锁保证线程安全


  // 插入到 bloomFilter, 方便快速进行判断
  bloomFilter_.Insert(simple_set_str);
  // 如果 key 已经存在, 那么根据自定义比较器应该会覆盖
  memMap_.insert(std::move(simple_set_str));

}

static thread_local std::string simple_get_str;

void Memtable::Get(const std::shared_ptr<GetContext> get_context) {
  // 使用 bloomFilter 进行快速判断
  if (false == bloomFilter_.IsMatch(get_context->key)) {
    get_context->code.setCode(StatusCode::kNotFound);
    LOG_INFO("from bloomfilter key: {} can not found", get_context->key);
    return;
  }

  simple_get_str = fmt::format("{}{}{}{}", kEmpty4Space, get_context->key,
                               kEmpty8Space, kEmpty1Space);
  // 赋值 key_size
  formatEncodeFixed32(get_context->key.size(), simple_get_str.data());
  // 赋值 number
  formatEncodeFixed64(get_context->number,
                      simple_get_str.data() + 4 + get_context->key.size());

  // 读写锁, 当多个线程共同读, 线程安全

  auto iter = memMap_.find(simple_get_str);


  if (iter == memMap_.end()) {
    LOG_WARN("key: {} can not find in memTable", get_context->key);
    get_context->code.setCode(StatusCode::kNotFound);
    return;
  }
  // 获取 key_size;
  uint32_t mem_key_size = formatDecodeFixed32(iter->data());

  // 获取 key, value 当前状态
  uint8_t key_type = formatDecodeFixed8(iter->data() + 4 + mem_key_size + 8);
  // key_type 处于已经被删除了, 直接返回即可
  if (key_type == ValueType::kTypeDeletion) {
    get_context->code.setCode(StatusCode::kNotFound);
    LOG_WARN("get key: {} has been delete", get_context->key);
    return;
  }

  // 获取 value_size;
  uint32_t mem_value_size =
      formatDecodeFixed32(iter->data() + mem_key_size + 4 + 9);

  // 获取 value_view
  std::string_view value_view(iter->data() + 4 + mem_key_size + 9 + 4,
                              mem_value_size);
  // 给 get_context 赋值
  get_context->value = value_view;
  get_context->code.setCode(StatusCode::kOk);
  LOG_INFO("get key: {} value: {} ok", get_context->key, value_view);
}

static thread_local std::string simple_del_str;

void Memtable::Delete(const std::shared_ptr<DeleteContext> del_context) {
  // 使用 bloomFilter 进行快速判断
  if (false == bloomFilter_.IsMatch(del_context->key)) {
    del_context->code.setCode(StatusCode::kNotFound);
    LOG_INFO("from bloomfilter delete key: {} can not found", del_context->key);
    return;
  }

  simple_del_str = fmt::format("{}{}{}{}", kEmpty4Space, del_context->key,
                               kEmpty8Space, kEmpty1Space);
  // 赋值 key_size
  formatEncodeFixed32(del_context->key.size(), simple_get_str.data());
  // 赋值 number
  formatEncodeFixed64(del_context->number,
                      simple_get_str.data() + 4 + del_context->key.size());
  // 这里也是查找行为

  auto iter = memMap_.find(simple_get_str);


  if (iter == memMap_.end()) {
    LOG_WARN("del key: {} can not find in memtable", del_context->key);
    del_context->code.setCode(StatusCode::kNotFound);
    return;
  }
  // 标记 删除
  // 获取 key_size;
  uint32_t mem_key_size = formatDecodeFixed32(iter->data());
  // 标志位删除.
  *(iter->data() + mem_key_size + 12) = ValueType::kTypeDeletion;
  LOG_INFO("del key: {} ok", del_context->key);
}

uint32_t Memtable::getMemSize() { return memMap_.size(); }

void Memtable::SetReadOnly() { isReadonly_ = true; }

void Memtable::SetNumber(const uint32_t number) {
  memtable_number_ = number;
}

}  // namespace db

}  // namespace fver