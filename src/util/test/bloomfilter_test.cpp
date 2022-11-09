#include "src/util/bloomfilter.hpp"
#include "src/base/log/logging.hpp"

#include <gtest/gtest.h>
#include <memory>
#include <uuid/uuid.h>


namespace fver {
namespace util {

class BloomFilterTest : public ::testing::Test {
 public:
  BloomFilterTest() = default;
  ~BloomFilterTest() = default;
  void SetUp() override {
    filter_ = std::make_shared<BloomFilter<>>();
  }
  void TearDown() override {

  }
  std::unordered_map<std::string, bool> map_;
  std::shared_ptr<BloomFilter<>> filter_;
};


TEST_F(BloomFilterTest, MultiKeyInsert) {
  const int N = 1000000;
  int i = 0;
  char key[64];
  char value[64];
  uuid_t uuid_key;
  uuid_t uuid_value;
  //
  //
  for (; i < N; i++) {
    uuid_generate(uuid_key);
    uuid_unparse(uuid_key, key);
    uuid_generate(uuid_value);
    uuid_unparse(uuid_value, value);
    map_.insert({key, true});
    filter_->Insert(key);
  }
  //
  for (auto& [key, value] : map_) {
    ASSERT_EQ(true, filter_->IsMatch(key));
  }
  //
  i = 0;
  for (; i < N; i++) {
    uuid_generate(uuid_key);
    uuid_unparse(uuid_key, key);
    if (true == filter_->IsMatch(key)) {
      LOG_INFO("uuid_key: {} error", key);
    }
    ASSERT_EQ(false, filter_->IsMatch(key));
  }
}



}  // namespace util

}  // namespace fver
