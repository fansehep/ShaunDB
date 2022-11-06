#include "src/util/file/appendfile.hpp"

#include <gtest/gtest.h>

#include "src/base/log/logging.hpp"

extern "C" {
#include <uuid/uuid.h>
}

namespace fver {

namespace util {

namespace file {

class AppendFileTest : public ::testing::Test {
 public:
  AppendFileTest() = default;
  ~AppendFileTest() = default;
  void SetUp() override {}
  void TearDown() override {}

 private:
};

TEST_F(AppendFileTest, simple_test) {
  AppendFile append_file;
  // append_file 初始化不用加上
  // 放在 home 目录试试正确性

  append_file.Init(".", "append_file_test.log");
  append_file.Clear();
  //
  const int N = 100000;
  int i = 0;
  char key[64] = {0};
  char value[64] = {0};
  uuid_t uuid_key;
  uuid_t uuid_value;
  // 用以校验数据
  std::vector<std::pair<std::string, std::string>> origin_for_test_Map_;
  for (; i < N; i++) {
    uuid_generate(uuid_key);
    uuid_generate(uuid_value);
    uuid_unparse(uuid_key, key);
    uuid_unparse(uuid_value, value);

    origin_for_test_Map_.push_back({key, value});
    if (i == 0 || i == 1) {
      LOG_INFO("key: {} value: {}", key, value);
    }
    append_file.Append(key, 36);
    append_file.Append(value, 36);
  }
  // 刷新缓冲区里的数据
  append_file.FlushBuffer();
  // 将文件系统中的数据全部落盘.
  append_file.Sync();
  i = 0;
  LOG_INFO("start read");
  std::string append_file_context;
  append_file.Read(&append_file_context);
  int j = 0;
  for (auto& [key, value] : origin_for_test_Map_) {
    auto key_size = key.size();
    auto value_size = value.size();
    // 文件 key 的 view.
    std::string_view file_key_view(append_file_context.data() + i, key_size);
    // 文件 value 的 view.
    std::string_view file_value_view(append_file_context.data() + i + key_size,
                                     value_size);
    j++;
    if (file_key_view != key) {
      LOG_INFO(
          "key: {} != file_key_view: {} i = {} key_size: {} value_size: {}",
          key, file_key_view, i, key_size, value_size);
    }
    if (file_value_view != value) {
      LOG_INFO(
          "value: {} != file_value_view: {} i = {} key_size: {} value_size: {}",
          value, file_value_view, i, key_size, value_size);
    }
    ASSERT_EQ(key, file_key_view);
    ASSERT_EQ(value, file_value_view);
    i = (i + key_size + value_size);
  }
}

}  // namespace file

}  // namespace util

}  // namespace fver