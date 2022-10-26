#include "src/server/redis_protocol.hpp"

#include <gtest/gtest.h>

#include "src/base/log/logging.hpp"

namespace fver {

namespace server {

class RedisProtoTest : public ::testing::Test {
 public:
  RedisProtoTest() = default;
  ~RedisProtoTest() = default;
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(RedisProtoTest, PING_TEST) {
  const std::string ping = "PING";
  auto result = fver::redis::parseRedisProtocol(ping.c_str(), ping.size());
  for (auto& str : result) {
    ASSERT_EQ(ping, str);
  }
}

TEST_F(RedisProtoTest, SET_TEST) {
  const std::string set_request(
      "*3\r\n$3\r\nSET\r\n$5\r\nmykey\r\n$7\r\nmyvalue\r\n");
  const std::vector<std::string> set_parse_request = {
      "3",
      "SET",
      "mykey",
      "myvalue",
  };
  auto result =
      fver::redis::parseRedisProtocol(set_request.c_str(), set_request.size());
#ifdef FVER_REDIS_PROTOCOL_DEBUG
  for (auto& iter : result) {
    fmt::print("{}\n", iter);
  }
#endif
  LOG_INFO("result len = {}", result.size());
  int i = 0;
  const int result_vec_n = set_parse_request.size();
  ASSERT_EQ(result_vec_n, result.size());
  for (; i < result_vec_n; i++) {
    ASSERT_EQ(set_parse_request[i], result[i]);
  }
}

TEST_F(RedisProtoTest, GET_TEST) {
  const std::string get_request("*2\r\n$3\r\nGET\r\n$7\r\nyou_key\r\n");
  const std::vector<std::string> get_parse_request = {
      "2",
      "GET",
      "you_key",
  };
  auto result =
      fver::redis::parseRedisProtocol(get_request.data(), get_request.size());
#ifdef FVER_REDIS_PROTOCOL_DEBUG
  for (auto& iter : result) {
    fmt::print("{}\n", iter);
  }
#endif
  LOG_INFO("result len = {}", result.size());
  int i = 0;
  const int result_vec_n = get_parse_request.size();
  for (; i < result_vec_n; i++) {
    ASSERT_EQ(get_parse_request[i], result[i]);
  }
}

}  // namespace server

}  // namespace fver