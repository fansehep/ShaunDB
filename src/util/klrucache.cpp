#include "src/util/klrucache.hpp"

#include "src/base/log/logging.hpp"

namespace fver {

namespace util {

bool kLRUCache::get(const std::string& key, std::string* value, bool* isErase) {
  assert(nullptr != value);
  assert(nullptr != isErase);
  auto cache_iter = cache_map_.find(key);
  if (cache_map_.end() == cache_iter) {
    return false;
  }
  // 当前 kv 记录已经被删除了
  // 直接返回即可
  if (cache_iter->second->isExist == false) {
    *isErase = true;
  } else {
    *isErase = false;
    *value = cache_iter->second->value;
  }
  const uint32_t cur_kv_size = key.size() + cache_iter->second->value.size();
  // 如果在 冷数据队列中 找到该 kv 记录
  // 放入 热数据队列中
  if (cache_iter->second->isHot == false) {
    CacheHandle cache(cache_iter->second, true);
    probation_list_.erase(cache_iter->second);
    protected_list_.push_front(cache);
    cache_map_[*(protected_list_.begin()->key)] = protected_list_.begin();
    // 冷数据容量需要相减
    // 热数据相加
    probation_memory_usage_ -= cur_kv_size;
    protected_memory_usage_ += cur_kv_size;
    // 当热数据数据过多时
    // push 到冷数据队列中
    while (protected_memory_usage_ >= max_protected_mem_size_) {
      assert(!protected_list_.empty());
      auto rese_iter = (protected_list_.rbegin());
      assert(rese_iter->key != nullptr);
      const auto rese_kv_size =
          (rese_iter->key->size()) + rese_iter->value.size();
      // 冷数据的一条记录
      CacheHandle t_cache(rese_iter, false);
      probation_list_.push_front(std::move(t_cache));
      cache_map_[*(probation_list_.begin()->key)] = probation_list_.begin();
      protected_list_.pop_back();
      // 热数据
      protected_memory_usage_ -= rese_kv_size;
      probation_memory_usage_ += rese_kv_size;
    }
    // 当冷数据过多时, pop 出去.
    while (probation_memory_usage_ >= max_probation_mem_size_) {
      auto rese_iter = probation_list_.rbegin();
      // map 先删除
      auto rese_kv_size = (rese_iter->key->size() + rese_iter->value.size());
      cache_map_.erase(*(rese_iter->key));
      probation_list_.pop_back();
      probation_memory_usage_ -= rese_kv_size;
    }
  } else {
    // 否则则是在 热数据队列中找到 该 kv 记录
    // 将该记录放入热数据队列头部中
    CacheHandle cache(cache_iter->second, true);
    cache_iter->second->key = &(cache_map_.find(key)->first);
    protected_list_.push_front(std::move(cache));
    cache_map_[*(protected_list_.begin()->key)] = protected_list_.begin();
    // 当热数据数据过多时
    // push 到冷数据队列中
    // 让冷数据队列 pop 出去
    while (protected_memory_usage_ >= max_protected_mem_size_) {
      auto rese_iter = protected_list_.rbegin();
      auto rese_kv_size = (rese_iter->key->size()) + rese_iter->value.size();
      // 冷数据的一条记录
      CacheHandle t_cache(rese_iter, false);
      probation_list_.push_front(std::move(t_cache));
      cache_map_[*(probation_list_.begin()->key)] = probation_list_.begin();
      protected_list_.pop_back();
      protected_memory_usage_ -= rese_kv_size;
    }
  }
  return true;
}

void kLRUCache::set(const std::string& key, const std::string& value) {
  CacheHandle cache(value);
  // 先插入到冷数据队列中
  probation_list_.push_front(std::move(cache));
  cache_map_[key] = probation_list_.begin();
  probation_list_.begin()->key = &(cache_map_.find(key)->first);
  probation_memory_usage_ += (key.size() + value.size());
  // 冷数据队列超过冷数据的最大使用容量
  while (probation_memory_usage_ >= max_probation_mem_size_) {
    auto rese_iter = probation_list_.rbegin();
    // map 先删除
    auto rese_kv_size = (rese_iter->key->size() + rese_iter->value.size());
    cache_map_.erase(*(rese_iter->key));
    probation_list_.pop_back();
    probation_memory_usage_ -= rese_kv_size;
  }
}

void kLRUCache::setEraseRecord(const std::string& key) {
  CacheHandle cache;
  cache.isExist = false;
  cache.isHot = false;
  probation_list_.push_front(cache);
  cache_map_[key] = probation_list_.begin();
  probation_list_.begin()->key = &(cache_map_.find(key)->first);
  probation_memory_usage_ += key.size();
  while (probation_memory_usage_ >= max_probation_mem_size_) {
    auto rese_iter = probation_list_.rbegin();
    auto rese_kv_size = (rese_iter->key->size() + rese_iter->value.size());
    // map 先删除
    cache_map_.erase(*(rese_iter->key));
    probation_list_.pop_back();
    probation_memory_usage_ -= rese_kv_size;
  }
}

uint32_t kLRUCache::getMemsize() {
  return probation_memory_usage_ + protected_memory_usage_;
}

auto kLRUCache::getMaxMemsize() -> uint32_t {
  return max_probation_mem_size_ + max_protected_mem_size_;
}

kLRUCache::kLRUCache(const uint32_t cold_memory_usage,
                     const uint32_t hot_memory_usage)
    : max_probation_mem_size_(cold_memory_usage),
      max_protected_mem_size_(hot_memory_usage),
      probation_memory_usage_(0),
      protected_memory_usage_(0) {
  // 让 hash_table 根据 cold_memory_usage + hot_memory_usage 直接扩容
  // 避免使用时 造成抖动.
  cache_map_.reserve((cold_memory_usage + hot_memory_usage) / 720);
}

void kLRUCache::Init(const uint32_t cold_memory_usage,
                     const uint32_t hot_memory_usage) {
  max_probation_mem_size_ = cold_memory_usage;
  max_protected_mem_size_ = hot_memory_usage;
  cache_map_.reserve((cold_memory_usage + hot_memory_usage) / 720);
}

}  // namespace util

}  // namespace fver