#ifndef SRC_UTIL_BLOOMFILTER_H_
#define SRC_UTIL_BLOOMFILTER_H_

#include "src/util/bitset.hpp"
#include "src/util/hash/city_hash.hpp"
#include "src/util/hash/farmhash.hpp"
#include "src/util/hash/xxhash64.hpp"
#include "src/base/log/logging.hpp"

namespace fver {
namespace util {

static constexpr int kBloomFilterDefaultSize = 16 * 1024 * 1024;

// 默认的使用 4 个哈希函数
template <int N = kBloomFilterDefaultSize>
class BloomFilter {
 public:
  BloomFilter()
      : seed_(rand()) {}

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
    auto idx_1 = XXHash64::hash(key.data(), key.length(), seed_);
    auto idx_2 = stdHash_(key);
    auto idx_3 = CityHash64WithSeed(key.data(), key.length(), seed_);
    auto idx_4 = ::util::Hash64WithSeed(key.data(), key.length(), seed_);
    LOG_INFO("idx_1: {} idx_2: {} idx_3: {} idx_4: {}", filter_.test(idx_1 % N),
             filter_.test(idx_2 % N), filter_.test(idx_3 % N),
             filter_.test(idx_4 % N));
    return false;
  }

  // 返回 bloom_filter 数据, 供 Compactor 做 Compaction.
  auto& getFilterData() { return filter_; }

 private:
  uint64_t seed_;
  std::hash<std::string_view> stdHash_;
  BitSet<N> filter_;
};

}  // namespace util
}  // namespace fver

#endif