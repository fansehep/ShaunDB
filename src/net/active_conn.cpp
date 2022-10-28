#include "src/net/active_conn.hpp"

extern "C" {
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
}
#include "src/base/log/logging.hpp"
#include "src/net/conn_callback.hpp"
#include "src/net/net_server.hpp"

namespace fver {

namespace net {

// 主动连接的回调函数应该不一样
void Connectioner::Init(const std::string& ip, const uint32_t port,
                        const std::shared_ptr<NetServer>& server, readHandle rh,
                        writeHandle wh, closeHandle ch, timeoutHandle th) {
  this->server_ = server;
  this->peer_ip_ = ip;
  this->peer_port_ = port;
  this->conn_.server_ = server.get();
  this->conn_.readHandle_ = rh;
  this->conn_.writeHandle_ = wh;
  this->conn_.closeHandle_ = ch;
  this->conn_.timeoutHandle_ = th;
  this->isRunning_ = false;
}

bool Connectioner::Run() {
  assert(nullptr != conn_.server_);
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(peer_port_);
  if (::inet_pton(AF_INET, peer_ip_.c_str(), &addr.sin_addr) <= 0) {
    LOG_ERROR("connect {} {} inet_pton error", peer_ip_, peer_port_);
  }
  conn_.socketFd_ = ::socket(
      addr.sin_family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
  auto ue =
      ::connect(conn_.socketFd_, reinterpret_cast<struct sockaddr*>(&addr),
                static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
  int saved_error = (ue == 0) ? 0 : errno;
  switch (saved_error) {
    case 0:
    case EINPROGRESS:
    case EINTR:
    case EISCONN: {
      this->conn_.Init();
      auto mtx = server_->getMutex();
      mtx->lock();
      server_->ConnectionMap_.insert({conn_.socketFd_, &conn_});
      mtx->unlock();
      isRunning_ = true;
      break;
    }

    case EAGAIN:
    case EADDRINUSE:
    case EADDRNOTAVAIL:
    case ECONNREFUSED:
    case ENETUNREACH:
    case EACCES:
    case EPERM:
    case EAFNOSUPPORT:
    case EALREADY:
    case EBADF:
    case EFAULT:
    case ENOTSOCK: {
      LOG_WARN("connect {} {} error", peer_ip_, peer_port_);
      ::close(conn_.socketFd_);
      return false;
    } break;

    default:
      break;
  }
  return true;
}

void Connectioner::CloseConn() {
  if (true == isRunning_) {
    ::close(conn_.socketFd_);
  }
}

Connectioner::~Connectioner() {
  CloseConn();
}

int Connectioner::getFd() { return conn_.socketFd_; }

const std::string& Connectioner::getPeerIP() { return peer_ip_; }

uint32_t Connectioner::getPort() { return peer_port_; }

}  // namespace net

}  // namespace fver