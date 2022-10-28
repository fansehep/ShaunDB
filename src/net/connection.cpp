#include "src/net/connection.hpp"

extern "C" {
#include <arpa/inet.h>
#include <event2/bufferevent.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <netinet/in.h>
}

#include "src/base/log/logging.hpp"
#include "src/net/net_server.hpp"

namespace fver {
namespace net {

void ConnectionReadCallback(struct bufferevent* buf, void* data) {
  auto conn = static_cast<Connection*>(data);
  struct evbuffer* bev = bufferevent_get_input(buf);
  int len = 0;
  while (true) {
    len = evbuffer_remove(bev, conn->readBuf_.bufptr_ + conn->readBuf_.offset_,
                          conn->readBuf_.buflen_);
    if (len <= 0) {
      break;
    }
    conn->readBuf_.offset_ += len;
  }
#ifdef FVER_NET_DEBUG
  LOG_INFO("Begin to readhanle1");
#endif
  if (conn->readHandle_) {
#ifdef FVER_NET_DEBUG
    // 处理数据
    LOG_INFO("Begin to readhanle2");
#endif
    auto simple_read_len =
        conn->readHandle_(conn->readBuf_.bufptr_, conn->readBuf_.offset_, conn);
    // 重置 readBuf_ 的偏移量
    // 如果 server 没读完, 则返回 -1, 然后保存本次数据, 下一次继续读完就行了.
    if (simple_read_len < 0) {
      conn->readBuf_.offset_ = 0;
    }
  }
}

void ConnectionWriteCallback(struct bufferevent* buf, void* data) {
  auto conn = static_cast<Connection*>(data);
#ifdef FVER_NET_DEBUG
  LOG_INFO("Begin to write handle1!");
#endif
  auto output = bufferevent_get_output(buf);
  if (evbuffer_get_length(output) == 0) {
    return;
  }
  if (conn->writeHandle_) {
    conn->writeHandle_(conn);
#ifdef FVER_NET_DEBUG
    LOG_INFO("Begin to write handle2!");
#endif
  }
}

void ConnectionEventCallback(struct bufferevent* buf, short eventWhat,
                             void* data) {
  auto conn = static_cast<Connection*>(data);
  if (eventWhat & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
    if (conn->closeHandle_) {
      conn->closeHandle_(conn);
    }
    bufferevent_free(buf);
    conn->server_->removeConn(conn->socketFd_);
  } else if (eventWhat & (BEV_EVENT_TIMEOUT)) {
    if (conn->timeoutHandle_) {
      conn->timeoutHandle_(conn);
    }
    bufferevent_free(buf);
    // 移除自己在 server 中的连接
    conn->server_->removeConn(conn->socketFd_);
  }
}

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
  bufferevent_setcb(buf_, ConnectionReadCallback, ConnectionWriteCallback,
                    ConnectionEventCallback, this);

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