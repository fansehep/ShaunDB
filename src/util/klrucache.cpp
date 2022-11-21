#include "src/util/klrucache.hpp"

namespace fver {

namespace util {

SingleCache::SingleCache(const uint32_t max_size)
    : memory_usage_(0), maxSize_(max_size) {}

bool SingleCache::get(const std::string &key, std::string *value,
                      bool *is_visit) {
  assert(value != nullptr);
  auto cache_iter = cache_map_.find(key);
  // 没有找到
  if (cache_map_.end() == cache_iter) {
    return false;
  }
  *value = cache_iter->second->value;
  cache_iter->second->is
  return true;
}

void SingleCache::set(const std::string &key, const std::string &value) {
  CacheHandle handle(value);
  cache_list_.push_front(std::move(handle));
  cache_map_[key] = cache_list_.begin();
  cache_list_.begin()->key =
      const_cast<std::string *>(&((cache_map_.find(key)->first)));
  memory_usage_ += (key.size() + value.size());
  if (memory_usage_ >= maxSize_) {
    // 移除 cache_list 尾部元素.
    auto reserve_iter = cache_list_.rbegin();
    auto kv_size = reserve_iter->key->size() + reserve_iter->value.size();
    cache_map_.erase(reserve_iter->key);
    cache_list_.pop_back();
    memory_usage_ -= kv_size;
  }
}

void SingleCache::setRemoveRecord(const std::string &key) {
  CacheHandle handle(key, false);
  cache_list_.push_front(std::move(handle));
  cache_map_[key] = cache_list_.begin();
  cache_list_.begin()->key =
      const_cast<std::string *>(&(cache_map_.find(key)->first));
  memory_usage_ += (key.size());
  if (memory_usage_ >= maxSize_) {
    // 移除 cache_list 尾部元素.
    auto reserve_iter = cache_list_.rbegin();
    auto kv_size = reserve_iter->key->size() + reserve_iter->value.size();
    cache_map_.erase(reserve_iter->key);
    cache_list_.pop_back();
    memory_usage_ -= kv_size;
  }
}

bool SingleCache::remove(const std::string &key) {
  auto cache_iter = cache_map_.find(key);
  if (cache_map_.end() == cache_iter) {
    return false;
  }
  cache_list_.erase(cache_iter->second);
  cache_map_[key] = cache_list_.end();
  return true;
}

}  // namespace util

}  // namespace fver