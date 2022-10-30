#include "src/net/connection.hpp"

#include <event2/buffer.h>
#include <event2/event.h>

extern "C" {
#include <arpa/inet.h>
#include <netinet/in.h>
}

#include "src/base/log/logging.hpp"
#include "src/net/net_server.hpp"

namespace fver {
namespace net {

Connection::Connection() : timeVal_({0, 0}) {}

Connection::Connection(evutil_socket_t socket, readHandle rh, writeHandle wh,
                       closeHandle ch, timeoutHandle th, NetServer* server)
    : readHandle_(rh),
      writeHandle_(wh),
      closeHandle_(ch),
      timeoutHandle_(th),
      socketFd_(socket),
      server_(server),
      timeVal_({0, 0}),
      conn_pair_({server_, socketFd_}) {}

bool Connection::Init() {
  buf_ = bufferevent_socket_new(server_->eventBase_, socketFd_,
                                BEV_OPT_CLOSE_ON_FREE);
  if (nullptr == buf_) {
    LOG_WARN("buf new socker error");
    return false;
  }

  bufferevent_setcb(buf_, callback::ConnectionReadCallback,
                    callback::ConnectionWriteCallback,
                    callback::ConnectionEventCallback, &conn_pair_);

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
  bufferevent_enable(buf_, EV_READ | EV_WRITE);
  return true;
}

bool Connection::getPeerConnInfo() {
  char ipAddr[32] = {0};
  struct sockaddr_in clientaddrinfo;
  socklen_t addr_len = sizeof(clientaddrinfo);
  std::memset(&clientaddrinfo, 0, sizeof(clientaddrinfo));

  if (::getpeername(socketFd_,
                    reinterpret_cast<struct sockaddr*>(&clientaddrinfo),
                    &addr_len)) {
    LOG_WARN("connection: fd: {}, getpeername error", socketFd_);
    return false;
  }

  if (::inet_ntop(AF_INET, &clientaddrinfo, ipAddr, sizeof(ipAddr)) ==
      nullptr) {
    LOG_WARN("connection: fd: {} inet_ntop error", socketFd_);
    return false;
  }
  peerPort_ = ::ntohs(clientaddrinfo.sin_port);
  peerIP_ = ::inet_ntoa(clientaddrinfo.sin_addr);
  return true;
}

Connection::~Connection() {
  ::bufferevent_free(this->buf_);
//  LOG_DEBUG("connection distruction!");
#ifdef FVER_NET_DEBUG
  LOG_INFO("Connection has be disconstruct!");
#endif
}

// int Connection::SendBuffer(ChannelBuffer* buf) {
//   assert(nullptr != buf_);
//   assert(nullptr != buf);
//   bufferevent_write(buf_, buf->getWritePtr(), buf->getReadablesize());
//   buf->clear();
//   return 1;
// }

// int Connection::AddbufToWriteBuf(evbuffer* buf) {
//   int len = 0;
//   while (true) {
//     len = evbuffer_remove(buf, input_buf_.getWritePtr(), input_buf_.getSize());
//     if (len <= 0) {
//       this->input_buf_.Resize(input_buf_.getRealSize() * 2);
//     } else {
//       break;
//     }
//   }
//   input_buf_.addSize(evbuffer_get_length(buf));
//   return 1;
// }

//   int Connection::ev_addbufToWriteBuf(struct ::evbuffer* buf) {
//     return evbuffer_add_buffer(ev_write_buf_, buf);
//   }


int Connection::Send(const char* msg, const size_t msg_size) {
  assert(nullptr != buf_);
  return bufferevent_write(buf_, msg, msg_size);
}

int Connection::SendEvWriteBuffer() {
  return bufferevent_write_buffer(buf_, ev_write_buf_);
}

void Connection::SetConnInithandle(connInitHandle conn_init_handle) {
  conninitHandle_ = conn_init_handle;
}

// int Connection::getInputBufferSize() {
//   return input_buf_.getSize();
// }


}  // namespace net

}  // namespace fver