#include "src/server/configparse.hpp"

#include <yaml-cpp/yaml.h>

#include "src/base/log/logger.hpp"
#include "src/server/server.hpp"
#include <charconv>

namespace fver {

namespace server {

const std::string kLog_STD_OUT = "STD_OUT";
const std::string kLog_Info = "Info";
const std::string kLog_Trace = "Trace";
const std::string kLog_Debug = "Debug";
const std::string kLog_Error = "Error";

bool ConfigParse(ServerConfig *config, const std::string &path) {
  auto yaml_doc = YAML::LoadFile(path);
  config->logpath = yaml_doc["log_config"]["log_path"].as<std::string>();
  std::string log_lev_str = yaml_doc["log_config"]["log_lev"].as<std::string>();
  if (log_lev_str == kLog_Info) {
    config->logLev = Logger::LogLevel::kInfo;
  } else if (log_lev_str == kLog_Trace) {
    config->logLev = Logger::LogLevel::kTrace;
  } else if (log_lev_str == kLog_Debug) {
    config->logLev = Logger::LogLevel::kDebug;
  } else if (log_lev_str == kLog_Error) {
    config->logLev = Logger::LogLevel::kError;
  } else if (log_lev_str == kLog_STD_OUT) {
    config->logLev = -1;
  } else {
    config->logLev = Logger::LogLevel::kInfo;
    return false;
  }
  config->log_name = yaml_doc["log_config"]["log_name"].as<std::string>();
  config->isSync = yaml_doc["log_config"]["log_issync"].as<bool>();
  config->listen_port = yaml_doc["server_config"]["listen_port"].as<uint32_t>();
  auto log_buf_size_str =
      yaml_doc["log_config"]["log_buf_size"].as<std::string>();
  std::string_view log_buf_size_view(log_buf_size_str.data(),
                                     log_buf_size_str.size() - 2);
  std::from_chars(log_buf_size_str.data(),
                  log_buf_size_str.data() + log_buf_size_str.size() - 2,
                  config->log_buf_size);
  config->log_buf_size = config->log_buf_size * 1024 * 1024;
  return true;
}

}  // namespace server

}  // namespace fver