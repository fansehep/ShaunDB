#include "src/util/bitset.hpp"

#include <gtest/gtest.h>

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
  BitSet<10240000> bitset_;
  int i = 0;
  std::vector<int> origin_for_test_vec;
  for (; i < 100000000; i++) {
    uint32_t idx = rand() % 1024;
    origin_for_test_vec.push_back(idx);
    bitset_.set(idx);
  }
  for (auto& iter : origin_for_test_vec) {
    ASSERT_EQ(bitset_.test(iter), true);
  }
}

}  // namespace util

}  // namespace fver