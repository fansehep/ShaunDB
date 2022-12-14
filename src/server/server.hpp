#ifndef SRC_SERVER_SERVER_H_
#define SRC_SERVER_SERVER_H_

#include "src/base/noncopyable.hpp"
#include "src/db/exportdb.hpp"
#include "src/net/connection.hpp"
#include "src/base/log/logbuffer.hpp"
#include <thread>
#include "src/net/net_server.hpp"

using ::fver::base::NonCopyable;
using ::fver::net::Connection;
using ::fver::net::NetServer;
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

namespace fver {

namespace server {


struct ServerConfig {
  std::string logpath;
  int logLev;
  bool isSync;
  std::string log_name;
  uint32_t log_buf_size;
  uint32_t listen_port;
};

class Server : public NonCopyable {
 public:
  Server() : buf_(1024) {}
  bool Init(const struct ServerConfig& conf);
  void Run();
  void Stop();

  int writeHD(const std::shared_ptr<Connection>& conn);
  int closeHD(const std::shared_ptr<Connection>& conn);
  int timeoutHD(const std::shared_ptr<Connection>& conn);
  // 如果消息没有读完, 请返回 -1
  int readHD(const std::shared_ptr<Connection>& conn);

 private:
  fver::base::log::Buffer buf_;
  std::thread net_server_thread_;
  fver::db::DB db_;
  NetServer server_;
};

}  // namespace server

}  // namespace fver

#endif