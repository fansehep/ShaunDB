#include <gtest/gtest.h>

#include "src/db/request.hpp"
extern "C" {
#include <uuid/uuid.h>
}
#include <charconv>
#include <string>

#include "src/base/log/logging.hpp"
#include "src/db/status.hpp"
#include "src/db/wal_writer.hpp"
#include "src/db/key_format.hpp"

namespace fver {

namespace db {

class WalLogTest : public ::testing::Test {
 public:
  WalLogTest() = default;
  ~WalLogTest() = default;
  void SetUp() override {}
  void TearDown() override {}
};

// 不超过 预写日志的最大容量进行写入
TEST_F(WalLogTest, log_write_test) {
  uint64_t number = 0;
  WalWriter writer;
  Status sts;
  writer.Init("./", "ShaunDB_WAL_LOG.log", &sts);
  if (sts.getCode() != kOk) {
    LOG_ERROR("can not init wal_log!");
  }
  const int curreny_N = 1000;
  int i = 0;
  char key[64] = {0};
  char value[64] = {0};
  uuid_t uuid_key;
  uuid_t uuid_value;
  std::unordered_map<std::string, std::string> origin_test_map;
  auto set_context = std::make_shared<SetContext>();
  std::string wal_simple_log;
  for (; i < curreny_N; i++) {
    uuid_generate(uuid_key);
    uuid_generate(uuid_value);
    uuid_unparse(uuid_key, key);
    uuid_unparse(uuid_value, value);
    set_context->key = key;
    set_context->value = value;

    SetContextWalLogFormat(set_context, number++, &wal_simple_log);
    // 解析出 crc32 校验和
    uint32_t crc32_check_sum = formatDecodeFixed32(wal_simple_log.data());
    // 解析出 序列号
    uint64_t check_number = formatDecodeFixed64(wal_simple_log.data() + 4);
    // 解析出 单条记录的
    uint8_t type = formatDecodeFixed8(wal_simple_log.data() + 12);
    uint32_t key_length = formatDecodeFixed32(wal_simple_log.data() + 13);
    uint32_t value_length = formatDecodeFixed32(wal_simple_log.data() + 17);
    std::string key_view(wal_simple_log.data() + 21, key_length);
    std::string value_view(wal_simple_log.data() + 21 + key_length,
                           value_length);
    std::string ksimple_log_end(
        wal_simple_log.data() + 21 + key_length + value_length,
        kFlagLogEnd.size());
    ASSERT_EQ(ksimple_log_end, kFlagLogEnd);
    LOG_INFO(
        "\n\t crc32_check_sum = {}\n\t check_number = {}\n\t type = {} \n\t "
        "key_length = {}\n\t value_length = {}\n\t key = {}\n\t value = {}",
        crc32_check_sum, check_number, type, key_length, value_length, key_view,
        value_view);
    ASSERT_EQ(key_view, set_context->key);
    ASSERT_EQ(value_view, set_context->value);
    ASSERT_EQ(check_number, number - 1);
    writer.AddRecord(wal_simple_log);
  }
}

}  // namespace db

}  // namespace fver