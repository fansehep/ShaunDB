#include "src/util/bitset.hpp"

#include <gtest/gtest.h>

#include <bitset>

namespace fver {

namespace util {

class BitSetTest : public ::testing::Test {
 public:
  BitSetTest() = default;
  ~BitSetTest() = default;
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(BitSetTest, simple_test) {
  const int N = 1024000;
  BitSet<N> bitset_;
  std::bitset<N> stl_bitset_;
  int i = 0;
  std::vector<uint32_t> vec_int_;
  for (; i < 1000; i++) {
    uint32_t idx = rand() % N;
    bitset_.set(idx);
    stl_bitset_.set(idx, true);
    vec_int_.push_back(idx);
  }
  i = 0;
  for (auto& iter : vec_int_) {
    ASSERT_EQ(bitset_.test(iter), stl_bitset_.test(iter));
  }
}

}  // namespace util

}  // namespace fver