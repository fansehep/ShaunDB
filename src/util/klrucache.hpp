#ifndef SRC_UTIL_LRUCACHE_
#define SRC_UTIL_LRUCACHE_

#include <absl/container/flat_hash_map.h>

#include <list>
#include <string>

namespace fver {

namespace util {

struct CacheHandle {
  std::string* key;
  std::string value;
  uint32_t visit_n;
  bool isExist;
  explicit CacheHandle(const std::string& v)
      : key(nullptr), value(v), visit_n(0), isExist(true) {}
  CacheHandle(const CacheHandle&& handle)
      : key(nullptr),
        value(std::move(handle.value)),
        visit_n(0),
        isExist(true) {}
  //
  explicit CacheHandle(const std::string& v, bool)
      : key(nullptr), value(v), visit_n(0), isExist(false) {}
};

class kLRUCache;

class SingleCache {
 public:
  friend kLRUCache;
  // @cache_size: 当前 cache 最大容量.
  explicit SingleCache(const uint32_t cache_size);
  /*
   * if get ok will return true
   * the value don't be nullptr
   * don't in cache will get the false
   */
  bool get(const std::string& key, std::string* value, bool* is_visit);

  // 插入一条 存在的 key value
  void set(const std::string& key, const std::string& value);

  // 插入一条被删除 key value
  void setRemoveRecord(const std::string& key);

  // 删除一条 key 记录
  bool remove(const std::string& key);

 private:
  // 当前的内存使用容量
  uint32_t memory_usage_;
  // singleCache 的最大值
  uint32_t maxSize_;
  // 链表, 存储 value
  std::list<CacheHandle> cache_list_;
  // hash O(1) 定位
  absl::flat_hash_map<std::string, std::list<CacheHandle>::iterator> cache_map_;
};

class kLRUCache {
 public:
  /*
   * @cold_data_usage: 冷数据占比
   * @max_memory_usage: 最大内存使用容量
   */
  explicit kLRUCache(const uint32_t cold_data_usage,
                     const uint32_t max_memory_usage);
};

}  // namespace util

}  // namespace fver

#endif