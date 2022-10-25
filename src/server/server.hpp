#ifndef SRC_SERVER_SERVER_H_
#define SRC_SERVER_SERVER_H_

#include "src/base/log/logging.hpp"
#include "src/base/noncopyable.hpp"
#include "src/db/exportdb.hpp"
#include "src/net/net_server.hpp"

using ::fver::base::NonCopyable;
using ::fver::net::NetServer;

namespace src {

namespace server {

struct ServerConfig {
  std::string logpath;
  int logLev;
  bool isSync;
  std::string log_name;
  uint32_t listen_port;
};

class Server : public NonCopyable {
 public:
  void Init(const ServerConfig& config);
  void Run();
  void Stop();
  // 我们不能直接使用他,
  // 我们应该选择写完之后触发他 
  int writeHD();
  int closeHD();
  int timeoutHD();
  // 如果消息没有读完, 请返回 -1
  int readHD();
 private:
  std::thread net_server_thread_;
  fver::db::DB db_;
  NetServer server_;
};

}  // namespace server

}  // namespace src

#endif