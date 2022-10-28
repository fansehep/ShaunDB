#include "src/net/connection.hpp"

extern "C" {
#include <arpa/inet.h>
#include <netinet/in.h>
}

#include "src/base/log/logging.hpp"
#include "src/net/net_server.hpp"

namespace fver {
namespace net {

Connection::Connection() : readBuf_(kConnectionBufferSize), timeVal_({0, 0}) {}

Connection::Connection(evutil_socket_t socket, readHandle rh, writeHandle wh,
                       closeHandle ch, timeoutHandle th, NetServer* server)
    : readHandle_(rh),
      writeHandle_(wh),
      closeHandle_(ch),
      timeoutHandle_(th),
      readBuf_(kConnectionBufferSize),
      socketFd_(socket),
      server_(server),
      timeVal_({0, 0}) {}

bool Connection::Init() {
  buf_ = bufferevent_socket_new(server_->eventBase_, socketFd_,
                                BEV_OPT_CLOSE_ON_FREE);
  if (nullptr == buf_) {
    LOG_WARN("buf new socker error");
    return false;
  }
  bufferevent_setcb(buf_, callback::ConnectionReadCallback,
                    callback::ConnectionWriteCallback,
                    callback::ConnectionEventCallback, this);

  if (timeVal_.tv_sec != 0 || timeVal_.tv_usec != 0) {
    struct timeval timeout = timeVal_;
    bufferevent_set_timeouts(buf_, &timeout, nullptr);
  }
  auto ue = getPeerConnInfo();
  if (false == ue) {
    LOG_WARN("fd: {} get peer_conn_error", this->socketFd_);
  } else {
    LOG_INFO("conn ip: {} port: {} create connection", this->peerIP_,
             this->peerPort_);
  }
  bufferevent_enable(buf_, EV_READ);
  return true;
}

bool Connection::getPeerConnInfo() {
  char ipAddr[16] = {0};
  struct sockaddr_in clientaddrinfo;
  socklen_t addr_len = sizeof(clientaddrinfo);
  std::memset(&clientaddrinfo, 0, sizeof(clientaddrinfo));

  if (getpeername(socketFd_,
                  reinterpret_cast<struct sockaddr*>(&clientaddrinfo),
                  &addr_len)) {
    LOG_WARN("connection: fd: {}, getpeername error", socketFd_);
    return false;
  }

  if (inet_ntop(AF_INET, &clientaddrinfo, ipAddr, sizeof(ipAddr)) == nullptr) {
    LOG_WARN("connection: fd: {} inet_ntop error", socketFd_);
    return false;
  }
  peerPort_ = ntohs(clientaddrinfo.sin_port);
  peerIP_ = ipAddr;
  return true;
}

Connection::~Connection() {
#ifdef FVER_NET_DEBUG
  LOG_INFO("Connection has be disconstruct!");
#endif
}

}  // namespace net

}  // namespace fver