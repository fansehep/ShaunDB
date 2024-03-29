#include "src/net/repeated_timer.hpp"

#include <functional>
#include <memory>

#include "src/base/log/logbuffer.hpp"
#include "src/base/log/logging.hpp"
#include "src/net/connection.hpp"
#include "src/net/net_server.hpp"

using ::fver::net::Connection;
using ::fver::net::NetServer;
using ::fver::net::RepeatedTimer;
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

namespace fver {

namespace net {

static const std::string kTimerHello =
    "the speed at which the cherry blossoms fall is 5cm/s\n";

class RepeatedServer {
 public:
  RepeatedServer() : buf_(1024) { server = std::make_shared<NetServer>(); }

  void Init(const uint32_t port) {
    server->Init(port, std::bind(&RepeatedServer::writeHd, this, _1),
                 std::bind(&RepeatedServer::closeHd, this, _1),
                 std::bind(&RepeatedServer::timeoutHd, this, _1),
                 std::bind(&RepeatedServer::readHd, this, _1));
    struct timeval time_val;
    time_val.tv_sec = 2;
    time_val.tv_usec = 0;
    timer_.Init(time_val, std::bind(&RepeatedServer::timerHD, this, _1), server,
                "for test");
    thread_ = std::thread([&]() { server->Run(); });
    timer_.Run();
  }

  int writeHd(const std::shared_ptr<Connection>& conn) {
    return 1;
  }

  int closeHd(const std::shared_ptr<Connection>& conn) {
    LOG_INFO("conn ip: {} port: {} close connection", conn->getPeerIP(),
             conn->getPeerPort());
    return 1;
  }

  int timeoutHd(const std::shared_ptr<Connection>& conn) {
    LOG_INFO("conn ip: {} port: {} connection time out", conn->getPeerIP(),
             conn->getPeerPort());
    return 1;
  }

  // TODO, if the data has be read ok, should return -1;
  int readHd(const std::shared_ptr<Connection>& conn) {
    assert(buf_.buflen_ >= 0);
    auto size =
        conn->moveEvReadBuffer(buf_.bufptr_ + buf_.offset_, buf_.buflen_);
    buf_.offset_ += size;
    std::string_view message(buf_.bufptr_, buf_.offset_);
    LOG_INFO("conn ip: {} port: {} send {}", conn->getPeerIP(),
             conn->getPeerPort(), message);
    conn->Send(buf_.bufptr_, buf_.offset_);
    buf_.offset_ = 0;
    return -1;
  }

  // default 给所有用户发送 ktimerhello
  int timerHD(RepeatedTimer* timer) {
    auto mtx = server->getMutex();
    mtx->lock();
    auto connMap = server->getConnMap();
    if (connMap.empty()) {
      mtx->unlock();
      return -1;
    }
    for (auto& [fd, conn] : connMap) {
      conn->Send(kTimerHello.c_str(), kTimerHello.size());
    }
    mtx->unlock();
    return 1;
  }

 private:
  std::thread thread_;
  base::log::Buffer buf_;
  std::shared_ptr<NetServer> server;
  RepeatedTimer timer_;
};

}  // namespace net
}  // namespace fver

int main() {
  fver::net::RepeatedServer server;
  LOG_INFO("server start 9090 port");
  server.Init(9090);
  LOG_INFO("server init successful");
  std::this_thread::sleep_for(std::chrono::seconds(60));
  return 0;
}