#ifndef SRC_UTIL_LRUCACHE_
#define SRC_UTIL_LRUCACHE_


#include <list>
#include <string>
#include <unordered_map>

namespace fver {

namespace util {

struct CacheHandle {
  const std::string* key;
  std::string value;
  // 是否处于热队列中
  //
  bool isHot;
  // kv 记录是否存在
  // 因为有可能插入一条被删除的kv记录
  // 由于 lsm_tree 的特殊性
  bool isExist;
  CacheHandle() = default;
  CacheHandle(const CacheHandle& ch)
      : key(ch.key), value(ch.value), isHot(ch.isHot), isExist(ch.isExist) {}
  explicit CacheHandle(const std::string& v)
      : key(nullptr), value(v), isHot(false), isExist(true) {}
  CacheHandle(CacheHandle&& handle)
      : key(handle.key),
        value(std::move(handle.value)),
        isHot(handle.isHot),
        isExist(handle.isExist) {}
  CacheHandle(std::list<CacheHandle>::iterator iter, bool hot)
      : key(iter->key),
        value(std::move(iter->value)),
        isHot(hot),
        isExist(iter->isExist) {}
  //
  CacheHandle(std::list<CacheHandle>::reverse_iterator iter, bool hot)
      : key(iter->key),
        value(std::move(iter->value)),
        isHot(hot),
        isExist(iter->isExist) {}

  explicit CacheHandle(const std::string& v, bool)
      : key(nullptr), value(v), isHot(0), isExist(false) {}
};

class kLRUCache {
 public:
  kLRUCache() = default;
  //
  void Init(const uint32_t cold_memory_usage, const uint32_t hot_memory_usage);
  /*
   * @cold_data_usage: 冷数据使用容量
   * @max_memory_usage: 最大内存使用容量
   */
  explicit kLRUCache(const uint32_t cold_memory_usage,
                     const uint32_t hot_memory_usage);
  /*
   * isErase 是否被删除
   *
   */
  bool get(const std::string& key, std::string* value, bool* isErase);

  //
  void set(const std::string& key, const std::string& value);

  // 加入一条被删除的记录
  void setEraseRecord(const std::string& key);

  // 获取当前lru的总容量
  uint32_t getMemsize();

  // 获取当前lru 的最大容量
  auto getMaxMemsize() -> uint32_t;

 private:
  // 冷数据的峰值容量
  uint32_t max_probation_mem_size_;
  // 热数据的峰值容量
  uint32_t max_protected_mem_size_;
  // 冷数据队列当前内存使用容量
  uint32_t probation_memory_usage_;
  // 热数据队列当前内存使用容量
  uint32_t protected_memory_usage_;
  // 热数据队列
  std::list<CacheHandle> protected_list_;
  // 冷数据队列
  std::list<CacheHandle> probation_list_;
  // cache map
  std::unordered_map<std::string, std::list<CacheHandle>::iterator> cache_map_;
};

}  // namespace util

}  // namespace fver

#endif