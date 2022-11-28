
#include "src/db/memtable_view.hpp"

#include <memory>

extern "C" {
#include <assert.h>
}

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

#ifdef DB_DEBUG

MemBTreeView* MemTable_view::getIndexView() { return &memMapView_; }

char* MemTable_view::getBloomFilterPtr() {
  return bloomFilter_->getFilterData()->getData();
}

uint32_t MemTable_view::getBloomFilterSize() {
  return bloomFilter_->getFilterData()->getSize();
}

#endif

MemTable_view::MemTable_view(const char* data, const uint32_t data_size,
                             const uint32_t n, const uint32_t lev)
    : cur_level_(lev), number_n_(n) {
  this->Init(std::string_view(data, data_size));
}

bool MemTable_view::Init(std::string_view mmap_view) {
  memViewPtr_ = mmap_view.data();
  memViewSize_ = mmap_view.size();
  uint32_t version;
  auto end_ptr = getVarint32Ptr(mmap_view.data(), (mmap_view.data() + 5), &version);
  // TODO: check it
  uint64_t create_time;
  end_ptr = getVarint64Ptr(end_ptr, end_ptr + 9, &create_time);
  uint32_t cur_lev;
  end_ptr = getVarint32Ptr(end_ptr, end_ptr + 5, &cur_lev);
  uint32_t cur_number;
  end_ptr = getVarint32Ptr(end_ptr, end_ptr + 5, &cur_number);
  uint32_t all_key_size;
  end_ptr = getVarint32Ptr(end_ptr, end_ptr + 5, &all_key_size);
  assert(cur_lev == cur_level_);
  assert(cur_number == cur_number);
  uint64_t bloomfilter_seed;
  end_ptr =
      getVarint64Ptr(end_ptr, mmap_view.data() + 5, &bloomfilter_seed);
  uint32_t bloomfilter_size;
  end_ptr = getVarint32Ptr(end_ptr, end_ptr + 4, &bloomfilter_size);
  bloomFilter_ = std::make_unique<util::BloomFilter<>>(
      end_ptr, bloomfilter_size * 8, bloomfilter_seed);
  end_ptr += bloomfilter_size;
  // 作为视图压入到 MemTable_view 中
  uint32_t key_size;
  uint32_t value_size;
  while (end_ptr < (mmap_view.data() + mmap_view.size())) {
    auto begin_ptr = end_ptr;
    end_ptr = getVarint32Ptr(end_ptr, end_ptr + 5, &key_size);
    auto end_2_ptr = end_ptr;
    assert(end_ptr);
    end_ptr += key_size;
    end_ptr = getVarint32Ptr(end_ptr, end_ptr + 5, &value_size);
    // 如果当前 kv 记录被删除, 那么就不会存储 value, 所以这里 getVarint32Ptr
    // 会返回 nullptr, 我们只需要保存 key_view 即可.
    if (end_ptr == nullptr) {
      std::string_view kv_str_view(end_2_ptr, key_size);
      memMapView_.insert(kv_str_view);
      continue;
    }
    end_ptr += value_size;
    std::string_view kv_str_view(begin_ptr, end_ptr);
    memMapView_.insert(kv_str_view);
  }
  assert(all_key_size == memMapView_.size());
  LOG_INFO("memtable_view: {} lev: {} construct ok size: {}", getNumber(),
           getLevel(), memMapView_.size());
  return true;
}

void MemTable_view::setNumber(const uint32_t n) { number_n_ = n; }

uint32_t MemTable_view::getNumber() { return number_n_; }

void MemTable_view::setLevel(const uint32_t n) { cur_level_ = n; }

uint32_t MemTable_view::getLevel() { return cur_level_; }

/*
  memtable_view kv_str_view: | key_size | key_view | value_size | value_view |
*/
void MemTable_view::Get(const std::shared_ptr<GetContext>& get_context) {
  if (false == bloomFilter_->IsMatch(get_context->key)) {
    get_context->code.setCode(StatusCode::kNotFound);
    LOG_INFO("memtable_view: {} lev: {} bloomFilter can not find: {} cur_memtable_size: {}",
             getNumber(), getLevel(), get_context->key, memMapView_.size());
    return;
  }
  auto pre_key_var_size = varintLength(get_context->key.size());
  std::string simple_get_str =
      fmt::format("{}{}", format32_vec[pre_key_var_size], get_context->key);
  encodeVarint32(simple_get_str.data(), get_context->key.size());

  auto find_iter = memMapView_.find(simple_get_str);
  if (find_iter == memMapView_.end()) {
    get_context->code.setCode(StatusCode::kNotFound);
    LOG_INFO("memtable_view: {} lev: {} can not find: {} ", getNumber(),
             getLevel(), get_context->key);
    return;
  }
  if (find_iter->size() == simple_get_str.size()) {
    get_context->code.setCode(StatusCode::kDelete);
    LOG_INFO("memtable_view: {} lev: {} find: {} deleted", getNumber(),
             getLevel(), get_context->key);
    return;
  }
  uint32_t key_size;
  auto end_ptr =
      getVarint32Ptr(find_iter->data(), find_iter->data() + 5, &key_size);
  assert(end_ptr != nullptr);
  end_ptr += key_size;
  uint32_t value_size;
  //
  end_ptr = getVarint32Ptr(end_ptr, end_ptr + 5, &value_size);
  get_context->value = std::string_view(end_ptr, value_size);
  get_context->code.setCode(StatusCode::kOk);
  LOG_INFO("memtable_view: {} lev: {} get key: {} vlaue: {} ok", getNumber(),
           getLevel(), get_context->key, get_context->value);
}

}  // namespace db

}  // namespace fver
