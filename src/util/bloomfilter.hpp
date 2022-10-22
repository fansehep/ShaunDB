#ifndef SRC_UTIL_BLOOMFILTER_H_
#define SRC_UTIL_BLOOMFILTER_H_


#include "src/util/hash/city_hash.hpp"
#include "src/util/hash/farmhash.hpp"
#include "src/util/hash/xxhash64.hpp"


namespace fver {
namespace util {

static constexpr int kBloomFilterDefaultSize = 8 * 1024 * 1024;

// 默认的使用 4 个哈希函数
template <int N = kBloomFilterDefaultSize>
class BloomFilter {
 public:
  BloomFilter()
      : seed_((static_cast<uint64_t>(rand()) << 32) |
              static_cast<uint64_t>(rand())) {}
  void Insert(const std::string_view& key) {
    auto idx_1 = XXHash64::hash(key.data(), key.length(), seed_);
    auto idx_2 = stdHash_(key);
    auto idx_3 = CityHash64WithSeed(key.data(), key.length(), seed_);
    auto idx_4 = ::util::Hash64WithSeed(key.data(), key.length(), seed_);
    filter_.set(idx_1 % N, true);
    filter_.set(idx_2 % N, true);
    filter_.set(idx_3 % N, true);
    filter_.set(idx_4 % N, true);
  }

  bool IsMatch(const std::string_view& key) {
    if (XXHash64::hash(key.data(), key.length(), seed_) && stdHash_(key) &&
        CityHash64WithSeed(key.data(), key.length(), seed_) &&
        ::util::Hash64WithSeed(key.data(), key.length(), seed_)) {
      return true;
    }
    return false;
  }

 private:
  uint64_t seed_;
  std::hash<std::string_view> stdHash_;
  std::bitset<N> filter_;
};

}  // namespace util
}  // namespace fver

#endif