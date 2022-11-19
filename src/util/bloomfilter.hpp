#ifndef SRC_UTIL_BLOOMFILTER_H_
#define SRC_UTIL_BLOOMFILTER_H_

#include "src/base/log/logging.hpp"
#include "src/util/bitset.hpp"
#include "src/util/hash/city_hash.hpp"
#include "src/util/hash/farmhash.hpp"
#include "src/util/hash/xxhash64.hpp"

namespace fver {
namespace util {
// 默认 bloom_filter 的数据大小是 1.2mb
static constexpr int kBloomFilterDefaultSize = 2 * 1024 * 1024;

// 默认的使用 4 个哈希函数
template <int N = kBloomFilterDefaultSize>
class BloomFilter {
 public:
  BloomFilter() : seed_(rand()) {}

  // 此构造函数用于一个bloomfilter_view 的构造.
  BloomFilter(const char* bitset_data, const uint32_t bitset_data_size,
              const uint64_t seed)
      : seed_(seed),
        filter_(const_cast<char*>(bitset_data), bitset_data_size) {}

  ~BloomFilter() = default;

  void Insert(const std::string_view& key) {
    auto idx_1 = XXHash64::hash(key.data(), key.length(), seed_);
    auto idx_2 = stdHash_(key);
    auto idx_3 = CityHash64WithSeed(key.data(), key.length(), seed_);
    auto idx_4 = ::util::Hash64WithSeed(key.data(), key.length(), seed_);
    filter_.set(idx_1 % N);
    filter_.set(idx_2 % N);
    filter_.set(idx_3 % N);
    filter_.set(idx_4 % N);
  }

  bool IsMatch(const std::string_view& key) {
    if (filter_.test(XXHash64::hash(key.data(), key.length(), seed_) % N) &&
        filter_.test(stdHash_(key) % N) &&
        filter_.test(CityHash64WithSeed(key.data(), key.length(), seed_) % N) &&
        filter_.test(::util::Hash64WithSeed(key.data(), key.length(), seed_) %
                     N)) {
      return true;
    }
    return false;
  }

  // 返回 bloom_filter 数据, 供 Compactor 做 Compaction.
  BitSet<N>* getFilterData() { return &filter_; }

  uint64_t getFilterSeed() { return seed_; }

  // 设置随机数
  void setSeed(const uint64_t seed) { seed_ = seed; }

 private:
  uint64_t seed_;
  std::hash<std::string_view> stdHash_;
  BitSet<N> filter_;
};

}  // namespace util
}  // namespace fver

#endif