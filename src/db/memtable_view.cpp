
#include "src/db/memtable_view.hpp"

#include <memory>

#include "src/db/key_format.hpp"
#include "src/db/request.hpp"
#include "src/db/status.hpp"
#include "src/util/bloomfilter.hpp"

namespace fver {

namespace db {

/*
| membloom_seed_ | membloom_size_ | bloom_filter_data |
| kv_data|
*/

MemTable_view::MemTable_view(const char* data, const uint32_t data_size) {
  this->Init(std::string_view(data, data_size));
}

bool MemTable_view::Init(std::string_view mmap_view) {
  uint64_t bloomfilter_seed;
  auto end_ptr =
      getVarint64Ptr(mmap_view.data(), mmap_view.data() + 5, &bloomfilter_seed);
  uint32_t bloomfilter_size;
  getVarint32Ptr(end_ptr, end_ptr + 4, &bloomfilter_size);
  //
  bloomFilter_ = std::make_unique<util::BloomFilter<>>(
      end_ptr, bloomfilter_size, bloomfilter_seed);
  // 作为视图压入到 MemTable_view 中
  uint32_t key_size;
  uint32_t value_size;
  while (end_ptr < (mmap_view.data() + mmap_view.size())) {
    auto begin_ptr = end_ptr;
    end_ptr = getVarint32Ptr(end_ptr, end_ptr + 5, &key_size);
    assert(end_ptr);
    end_ptr += key_size;
    auto mid_ptr = end_ptr;
    end_ptr = getVarint32Ptr(end_ptr, end_ptr + 5, &value_size);
    // 如果当前 kv 记录被删除, 那么就不会存储 value, 所以这里 getVarint32Ptr
    // 会返回 nullptr, 我们只需要保存 key_view 即可.
    if (end_ptr == nullptr) {
      std::string_view kv_str_view(begin_ptr, mid_ptr);
      memMapView_.insert(kv_str_view);
      continue;
    }
    end_ptr += value_size;
    std::string_view kv_str_view(begin_ptr, end_ptr);
    memMapView_.insert(kv_str_view);
  }
  LOG_INFO("memtable_view construct ok size: {}", memMapView_.size());
  return true;
}

/*
  memtable_view kv_str_view: | key_size | key_view | value_size | value_view |
*/
void MemTable_view::Get(const std::shared_ptr<GetContext>& get_context) {
  if (false == bloomFilter_->IsMatch(get_context->key)) {
    get_context->code.setCode(StatusCode::kNotFound);
    LOG_INFO("memtable_view bloomFilter can not find: {}", get_context->key);
    return;
  }
  auto pre_key_var_size = varintLength(get_context->key.size());
  std::string simple_get_str =
      fmt::format("{}{}", format32_vec[pre_key_var_size], get_context->key);
  encodeVarint32(simple_get_str.data(), get_context->key.size());

  auto find_iter = memMapView_.find(simple_get_str);
  if (find_iter == memMapView_.end()) {
    get_context->code.setCode(StatusCode::kNotFound);
    LOG_INFO("memtable_view can not find: {} ", get_context->key);
    return;
  }
  if (find_iter->size() == simple_get_str.size()) {
    get_context->code.setCode(StatusCode::kDelete);
    LOG_INFO("memtable_view find: {} deleted", get_context->key);
    return;
  }
  uint32_t key_size;
  auto end_ptr =
      getVarint32Ptr(find_iter->data(), find_iter->data() + 5, &key_size);
  assert(end_ptr != nullptr);
  end_ptr += key_size;
  uint32_t value_size;
  //
  getVarint32Ptr(end_ptr, end_ptr + 5, &value_size);
  get_context->value = std::string_view(end_ptr, value_size);
  get_context->code.setCode(StatusCode::kOk);
  LOG_INFO("get key: {} vlaue: {} ok", get_context->key, get_context->value);
}

}  // namespace db

}  // namespace fver