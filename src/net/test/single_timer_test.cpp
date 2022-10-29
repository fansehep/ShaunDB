#include "src/net/single_timer.hpp"

#include <functional>
#include <memory>

#include "src/base/log/logging.hpp"
#include "src/net/connection.hpp"
#include "src/net/net_server.hpp"

using ::fver::net::Connection;
using ::fver::net::NetServer;
using ::fver::net::SingleTimer;
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

namespace fver {

namespace net {

static const std::string kTimerHello =
    "the speed at which the cherry blossoms fall is 5cm/s\n";

class SingleServer {
 public:
  SingleServer() : buf_(1024) { server = std::make_shared<NetServer>(); }

  void Init(const uint32_t port) {
    server->Init(port, std::bind(&SingleServer::writeHd, this, _1),
                 std::bind(&SingleServer::closeHd, this, _1),
                 std::bind(&SingleServer::timeoutHd, this, _1),
                 std::bind(&SingleServer::readHd, this, _1, _2, _3));
    struct timeval time_val;
    time_val.tv_sec = 5;
    time_val.tv_usec = 10;
    timer_.Init(time_val, std::bind(&SingleServer::timerHD, this, _1), server,
                "for test");
    thread_ = std::thread([&]() { server->Run(); });
    timer_.Run();
  }

  int writeHd(const std::shared_ptr<Connection>& conn) {
    std::string_view message(buf_.bufptr_, buf_.offset_);
    LOG_INFO("server send {}", message);
    conn->Send(buf_.bufptr_, buf_.offset_);
    buf_.offset_ = 0;
    return 1;
  }

  int closeHd(const std::shared_ptr<Connection>& conn) {
    LOG_INFO("conn ip: {} port: {} close connection", conn->getPeerIP(),
             conn->getPeerPort());
    return 1;
  }

  int timeoutHd(const std::shared_ptr<Connection>& conn) {
    LOG_WARN("conn ip: {} port: {} connection time out", conn->getPeerIP(),
             conn->getPeerPort());
    return 1;
  }

  // TODO, if the data has be read ok, should return -1;
  int readHd(char* buf, size_t size, const std::shared_ptr<Connection>& conn) {
    assert(buf_.buflen_ >= 0);
    std::string_view message(buf, size);
    std::memcpy(buf_.bufptr_, buf, size);
    buf_.offset_ += size;
    LOG_INFO("conn ip: {} port: {} send {}", conn->getPeerIP(),
             conn->getPeerPort(), message);
    conn->Send(buf, size);
    return -1;
  }

  // default 给所有用户发送 ktimerhello
  int timerHD(SingleTimer* timer) {
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
  Buffer buf_;
  std::shared_ptr<NetServer> server;
  SingleTimer timer_;
};

}  // namespace net
}  // namespace fver

int main() {
  fver::net::SingleServer server;
  LOG_INFO("singleserver start 9090 port");
  server.Init(9090);
  LOG_INFO("singleserver init successful");
  std::this_thread::sleep_for(std::chrono::seconds(60));
  return 0;
}