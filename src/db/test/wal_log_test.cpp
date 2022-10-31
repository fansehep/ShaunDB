#include <gtest/gtest.h>
#include <uuid/uuid.h>

#include "src/base/log/logging.hpp"
#include "src/db/status.hpp"
#include "src/db/wal_writer.hpp"

namespace fver {

namespace db {

class WalLogTest : public ::testing::Test {
 public:
  WalLogTest() = default;
  ~WalLogTest() = default;
  void SetUp() override {}
  void TearDown() override {
  }
};

const std::string kLog_Simple = "key_size:8key:12312312value_size:4value:1234";

// 测试日志写入之后是否会seek 到开始并且继续写入
TEST_F(WalLogTest, log_write_test) {
  WalWriter writer;
  writer.SetWalLogDefultSize(128);
  Status status;
  writer.Init("./", "wal_log_test.log", &status);
  if (status.getCode() != StatusCode::kOk) {
    LOG_ERROR("can not open the file");
    ::exit(-1);
  }
  char buf[168] = {0};
  for (int i = 1; i < 18; i++) {
    writer.AddRecord(kLog_Simple);
    std::memset(buf, 0, sizeof(buf));
    auto log_size = writer.ReadLog(buf, sizeof(buf));
    LOG_TRACE("wal_log size: {} context: {}", log_size, buf);
  }
}

// 测试恢复日志, 没有写满的情况下进行恢复
// 假设日志格式是 | sequence_number 8bit | operator_type 1bit | key_size 4bit |
// value_size 4bit | key | value | kflagend |
TEST_F(WalLogTest, log_restart_test) {
  uint64_t index = 0;
  WalWriter writer;
  writer.SetWalLogDefultSize(1024 * 20);
  Status status;
  writer.Init("./", "wal_log_test_restart.log", &status);

  if (status.getCode() != StatusCode::kOk) {
    LOG_ERROR("can not open the file");
    ::exit(-1);
  }

  const int N = 10;
  char key[64];
  char value[64];
  uuid_t uuid_key;
  uuid_t uuid_value;
  int i = 0;
  std::map<std::string, std::string> origin_storgae_;
  std::string pre_simple_log;
  for (; i < N; i++) {
    uuid_generate(uuid_key);
    uuid_unparse(uuid_key, key);
    uuid_generate(uuid_value);
    uuid_unparse(uuid_value, value);
    uint32_t key_rand = (rand() % 64);
    uint32_t value_rand = (rand() % 64);
    std::string_view key_view(key, key_rand);
    std::string_view value_view(value, value_rand);
    origin_storgae_.insert({key_view.data(), value_view.data()});
    char oper_temp = 1;
    auto simple_log =
        fmt::format("{:0>8}{:0>1}{:0>4}{:0>4}{}{}{}", index++, oper_temp,
                    key_rand, value_rand, key_view, value_view, kFlagLogEnd);
    if (i == 0) {
      pre_simple_log = std::string(simple_log.data(), simple_log.size()-4);
      LOG_TRACE("simple_log: {}", pre_simple_log);
    }
    writer.AddRecord(simple_log);
  }
  char buf[20480] = {0};
  const auto buf_size = sizeof(buf);
  auto read_size = writer.ReadLog(buf, buf_size);
  LOG_INFO("read_size: {} buf_size: {}", read_size, buf_size);
  std::string_view buf_view(buf, 2048);
  i = 0;
  for (; i < buf_size; i++) {
    auto t_index = buf_view.find(kFlagLogEnd, i);
    if (std::string_view::npos == t_index) {
      LOG_ERROR("can not find the std::string::npos");
      break;
    }
    std::string_view value(buf + i, t_index);
    LOG_DEBUG("parse value = {}", value);
    if (value != pre_simple_log) {
      LOG_ERROR("value != pre_simple_log");
    }
    break;
  }
}

}  // namespace db

}  // namespace fver