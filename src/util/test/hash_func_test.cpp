#include <gtest/gtest.h>
#include <uuid/uuid.h>

#include "src/util/hash/city_hash.hpp"
#include "src/util/hash/farmhash.hpp"
#include "src/util/hash/xxhash64.hpp"

namespace fver {

namespace util {

class HashFuncTest : public ::testing::Test {
 public:
  HashFuncTest() = default;
  ~HashFuncTest() = default;
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(HashFuncTest, multi_hash_func_test) {
  int i = 0;
  int j = 0;
  int seed_ = static_cast<uint64_t>(rand()) | static_cast<uint64_t>(rand());
  char key[64];
  char value[64];
  uuid_t uuid_key;
  uuid_t uuid_value;
  std::hash<std::string_view> stdHash_;
  const int N = 4;
  for (; i < N; i++) {
    uuid_generate(uuid_key);
    uuid_unparse(uuid_key, key);
    uuid_generate(uuid_value);
    uuid_unparse(uuid_value, value);
    std::string_view key_view(key, 36);

    auto idx_1 = XXHash64::hash(key_view.data(), key_view.length(), seed_);
    auto idx_2 = stdHash_(key);
    auto idx_3 = CityHash64WithSeed(key_view.data(), key_view.length(), seed_);
    auto idx_4 =
        ::util::Hash64WithSeed(key_view.data(), key_view.length(), seed_);

    // 等待 1 s
    std::this_thread::sleep_for(std::chrono::seconds(20));

    auto idx2_1 = XXHash64::hash(key_view.data(), key_view.length(), seed_);
    auto idx2_2 = stdHash_(key);
    auto idx2_3 = CityHash64WithSeed(key_view.data(), key_view.length(), seed_);
    auto idx2_4 =
        ::util::Hash64WithSeed(key_view.data(), key_view.length(), seed_);

    ASSERT_EQ(idx_1, idx2_1);
    ASSERT_EQ(idx_2, idx2_2);
    ASSERT_EQ(idx_3, idx2_3);
    ASSERT_EQ(idx_4, idx2_4);
  }
}

}  // namespace util

}  // namespace fver