#include <gtest/gtest.h>

#include "src/db/compactor.hpp"
#include "src/db/key_format.hpp"

namespace fver {

namespace db {

class CompactionTest : public ::testing::Test {
 public:
  CompactionTest() = default;
  ~CompactionTest() = default;
  void SetUp() override {}
  void TearDown() override {}
};

// 测试16个 key_value 前缀压缩
TEST_F(CompactionTest, prefix16_keyvalue) {
  const std::string key_1 = "abcdef_123123123";
  const std::string key_2 = "abcd123";
  const std::string key_3 = "abc1231231";
  //
  const std::string value_1 = "abcjdiasdj";
  const std::string value_2 = "abcjdiasdj";
  const std::string value_3 = "abcjdiijlajida";
  std::vector<SSTableKeyValueStyle> sstable_vec;
  SSTableKeyValueStyle kv_style;
  kv_style.key_view = key_1;
  kv_style.value_view = value_1;
  sstable_vec.push_back(kv_style);
  kv_style.key_view = key_2;
  kv_style.value_view = value_2;
  sstable_vec.push_back(kv_style);
  kv_style.key_view = key_3;
  kv_style.value_view = value_3;
  sstable_vec.push_back(kv_style);
  auto prefix16_key_value = Format16PrefixStr(sstable_vec, nullptr);
  LOG_INFO("prefix16_key_value key_view: {} value_view: {}",
           prefix16_key_value.key_view, prefix16_key_value.value_view);
  ASSERT_EQ(prefix16_key_value.key_view, "abc");
  ASSERT_EQ(prefix16_key_value.value_view, "abcjdi");
}

TEST_F(CompactionTest, no_prefix_key_value) {
  const std::string key_1 = "123123dajsikdjiu23hrui";
  const std::string value_1 = "djiahfukrsjhgfiurhugf";
  const std::string key_2 = "dhuaskchusrhf";
  const std::string value_2 = "123djkadukehf";
  const std::string key_3 = "mmmzzzdhuakehdu";
  const std::string value_3 = "zzzzdjilfjsdikf";
  std::vector<SSTableKeyValueStyle> sstable_vec;
  SSTableKeyValueStyle kv_style;
  kv_style.key_view = key_1;
  kv_style.value_view = value_1;
  sstable_vec.push_back(kv_style);
  kv_style.key_view = key_2;
  kv_style.value_view = value_2;
  sstable_vec.push_back(kv_style);
  kv_style.key_view = key_3;
  kv_style.value_view = value_3;
  sstable_vec.push_back(kv_style);
  auto prefix16_key_value = Format16PrefixStr(sstable_vec, nullptr);
  LOG_INFO("prefix16_key_value key_view: {} value_view: {}",
           prefix16_key_value.key_view, prefix16_key_value.value_view);
  ASSERT_EQ(prefix16_key_value.key_view.empty(), true);
  ASSERT_EQ(prefix16_key_value.value_view.empty(), true);
}
}  // namespace db

}  // namespace fver