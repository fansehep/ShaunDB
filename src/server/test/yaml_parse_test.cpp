#include <gtest/gtest.h>

#include "src/server/configparse.hpp"
#include "src/server/server.hpp"

namespace fver {

namespace server {

// ./bazel-bin/src/server/test/yaml_parse_test
//
class YamlParseTest : public ::testing::Test {
 public:
  YamlParseTest() = default;
  ~YamlParseTest() = default;
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(YamlParseTest, simple_config_test) {
  struct ServerConfig conf;
  ConfigParse(&conf, "./src/server/test/yaml_parse_test.yaml");
  LOG_INFO(
      "conf.isSync: {} \n conf.log_name: {} \n conf.logpath: {} \n "
      "conf.loglev: {} \n conf.listen_port: {}",
      conf.isSync, conf.log_name, conf.logpath, conf.logLev, conf.listen_port);
  ASSERT_EQ(conf.isSync, false);
  ASSERT_EQ(conf.log_name, "yaml_test");
  ASSERT_EQ(conf.logpath, "./");
  ASSERT_EQ(conf.logLev, Logger::LogLevel::kInfo);
  ASSERT_EQ(conf.listen_port, 9090);
}

}  // namespace server

}  // namespace fver