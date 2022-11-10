#include <gtest/gtest.h>

#include "src/base/log/logging.hpp"
#include "src/db/key_format.hpp"

namespace fver {

namespace db {

class VarintEncodeTest : public ::testing::Test {
 public:
  VarintEncodeTest() = default;
  ~VarintEncodeTest() = default;
  void SetUp() override {}
  void TearDown() override {}

 private:
};

TEST_F(VarintEncodeTest, test_encode_decode) {
  std::vector<int32_t> int32_vec;
  int i = 0;
  const int N = 100;
  for (; i < N; i++) {
    int32_vec.push_back(static_cast<int32_t>(rand()));
  }
  char buf[500] = {0};
  auto start_ptr = buf;
  for (auto& iter : int32_vec) {
    start_ptr = encodeVarint32(start_ptr, iter);
  }
  start_ptr = buf;
  char* end_ptr = start_ptr + sizeof(buf);
  i = 0;
  while (true) {
    uint32_t value;
    auto result_ptr = getVarint32Ptr(start_ptr, end_ptr, &value);
    if (result_ptr != nullptr) {
      auto varint_len = varintLength(value);
      start_ptr += varint_len;
      ASSERT_EQ(value, int32_vec[i]);
      LOG_INFO("value: {}, int32_vec[{}]: {}, varint_len: {}", value, i,
               int32_vec[i], varint_len);
      i++;
    } else {
      LOG_INFO("i = {}", i);
      break;
    }
  }
}

}  // namespace db

}  // namespace fver