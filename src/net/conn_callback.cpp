#include "src/net/conn_callback.hpp"

#include "src/base/log/logging.hpp"
#include "src/net/connection.hpp"
#include "src/net/net_server.hpp"

extern "C" {
#include <arpa/inet.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <netinet/in.h>
#include <sys/socket.h>
}

namespace fver {

namespace net {

namespace callback {

void ConnectionReadCallback(struct bufferevent* buf, void* data) {
  auto conn_pair = reinterpret_cast<std::pair<NetServer*, int>*>(data);
  auto conn = conn_pair->first->getConn(conn_pair->second);
  struct evbuffer* bev = ::bufferevent_get_input(buf);
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

void ConnectionWriteCallback(struct ::bufferevent* buf, void* data) {
  auto conn_pair = reinterpret_cast<std::pair<NetServer*, int>*>(data);
  auto conn = conn_pair->first->getConn(conn_pair->second);
#ifdef FVER_NET_DEBUG
  LOG_INFO("Begin to write handle1!");
#endif
  auto output = ::bufferevent_get_output(buf);
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

void ConnectionEventCallback(struct ::bufferevent* buf, short eventWhat,
                             void* data) {
  auto conn_pair = reinterpret_cast<std::pair<NetServer*, int>*>(data);
  auto conn = conn_pair->first->getConn(conn_pair->second);
  if (eventWhat & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
    if (conn->closeHandle_) {
      conn->closeHandle_(conn);
    }
    conn->server_->removeConn(conn->socketFd_);
    return;
  } else if (eventWhat & (BEV_EVENT_TIMEOUT)) {
    if (conn->timeoutHandle_) {
      conn->timeoutHandle_(conn);
    }
    // 移除自己在 server 中的连接
    conn->server_->removeConn(conn->socketFd_);
    return;
  }
}

}  // namespace callback

}  // namespace net

}  // namespace fver