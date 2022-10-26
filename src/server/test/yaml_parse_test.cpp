#include <gtest/gtest.h>

#include "src/server/configparse.hpp"
#include "src/server/server.hpp"

namespace fver {

namespace server {

class YamlParseTest : public ::testing::Test {
 public:
  YamlParseTest() = default;
  ~YamlParseTest() = default;
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(YamlParseTest, simple_config_test) {
  struct ServerConfig conf;
  ConfigParse(&conf, "./yaml_parse_test.yaml");
  ASSERT_EQ(conf.isSync, false);
  ASSERT_EQ(conf.log_name, "yaml_test");
  ASSERT_EQ(conf.logpath, "./");
  ASSERT_EQ(conf.logLev, Logger::LogLevel::kInfo);
  ASSERT_EQ(conf.listen_port, 9090);
}

}  // namespace server

}  // namespace fver