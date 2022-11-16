#include <gtest/gtest.h>

#include "src/db/compactor.hpp"
#include "src/db/exportdb.hpp"
#include "src/db/key_format.hpp"
#include "src/db/request.hpp"

extern "C" {
#include <uuid/uuid.h>
}

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
  auto prefix16_key_value = Format16PrefixStr(sstable_vec);
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
  auto prefix16_key_value = Format16PrefixStr(sstable_vec);
  LOG_INFO("prefix16_key_value key_view: {} value_view: {}",
           prefix16_key_value.key_view, prefix16_key_value.value_view);
  ASSERT_EQ(prefix16_key_value.key_view.empty(), true);
  ASSERT_EQ(prefix16_key_value.value_view.empty(), true);
}

TEST_F(CompactionTest, sstable_transform_test) {
  Memtable mem_table;
  const std::string key_1 = "123123dajsikdjiu23hrui";
  const std::string value_1 = "djiahfukrsjhgfiurhugf";
  auto set_context = std::make_shared<SetContext>();
  set_context->key = key_1;
  set_context->value = value_1;
  mem_table.Set(set_context);
  auto iter = mem_table.getMemTableRef().begin();
  auto mem_table_ref = mem_table.getMemTableRef();
  for (auto& iter : mem_table_ref) {
    LOG_INFO("iter: {} origin_size: {}", iter, key_1.size() + value_1.size());
    for (int i = 0; i < iter.size(); i++) {
      fmt::print("{}: {}\n", i, iter[i]);
    }
  }
  LOG_INFO("iter: {}", iter->c_str());
  auto kv_style = formatMemTableToSSTable(iter);
  //
  ASSERT_EQ(kv_style.key_view, key_1);
  ASSERT_EQ(kv_style.value_view, value_1);
  ASSERT_EQ(kv_style.isExist, true);
  //
  auto del_context = std::make_shared<DeleteContext>();
  //
  del_context->key = key_1;
  LOG_INFO("begin delete");
  mem_table.Delete(del_context);
  LOG_INFO("delete end");
  auto iter_2 = mem_table.getMemTableRef().begin();
  auto kv_style_2 = formatMemTableToSSTable(iter_2);
  ASSERT_EQ(kv_style_2.key_view, key_1);
  ASSERT_EQ(kv_style_2.value_view, value_1);
  ASSERT_EQ(kv_style_2.isExist, false);
}



}  // namespace db

}  // namespace fver