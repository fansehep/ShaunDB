#include "src/server/configparse.hpp"
#include "src/server/server.hpp"

int main(int argc, char** argv) {
  fver::server::ServerConfig config;
  // 使用默认参数
  if (argc == 1) {
    config.isSync = false;
    config.listen_port = 9090;
    config.log_name = "test";
    config.logLev = -1;
    config.logpath = "./";
  } else {
    fver::server::ConfigParse(&config, argv[1]);
  }
  fver::server::Server server;
  server.Init(config);
  server.Run();
  return 0;
}
