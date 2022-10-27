#include <functional>

#include "src/base/log/logging.hpp"
#include "src/net/connection.hpp"
#include "src/net/net_server.hpp"

using ::fver::net::Connection;
using ::fver::net::NetServer;
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

namespace fver {

namespace net {

class EchoServer {
 public:
  EchoServer() : buf_(1024) {}

  void Init(const uint32_t port) {
    server.Init(port, std::bind(&EchoServer::writeHd, this, _1),
                std::bind(&EchoServer::closeHd, this, _1),
                std::bind(&EchoServer::timeoutHd, this, _1),
                std::bind(&EchoServer::readHd, this, _1, _2, _3));
    server.Run();
  }

  int writeHd(Connection* conn) {
    std::string_view message(buf_.bufptr_, buf_.offset_);
    LOG_INFO("server send {}", message);
    conn->Send(buf_.bufptr_, buf_.offset_);
    buf_.offset_ = 0;
    return 1;
  }

  int closeHd(Connection* conn) {
    LOG_INFO("conn ip: {} port: {} close connection", conn->getPeerIP(),
             conn->getPeerPort());
    return 1;
  }

  int timeoutHd(Connection* conn) {
    LOG_INFO("conn ip: {} port: {} connection time out", conn->getPeerIP(),
             conn->getPeerPort());
    return 1;
  }

  // TODO, if the data has be read ok, should return -1;
  int readHd(char* buf, size_t size, Connection* conn) {
    assert(buf_.buflen_ >= 0);
    std::string_view message(buf, size);
    std::memcpy(buf_.bufptr_, buf, size);
    buf_.offset_ += size;
    LOG_INFO("conn ip: {} port: {} send {}", conn->getPeerIP(),
             conn->getPeerPort(), message);
    conn->Send(buf, size);
    return -1;
  }

 private:
  Buffer buf_;
  NetServer server;
};

}  // namespace net
}  // namespace fver

int main() {
  fver::net::EchoServer server;
  LOG_INFO("server start 9090");
  server.Init(9090);
  LOG_INFO("server disable");
  return 0;
}