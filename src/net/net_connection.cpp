#include "src/net/net_connection.hpp"

#include <event2/bufferevent.h>

#include "src/net/net_server.hpp"

namespace fver {
namespace net {

void ConnectionReadCallback(struct bufferevent* buf, void* data) {
  auto conn = static_cast<Connection*>(data);
  size_t len = 0;
  for (;;) {
    len = bufferevent_read(buf, (conn->buf_.bufptr_ + conn->buf_.offset_),
                           conn->buf_.buflen_ - conn->buf_.offset_);
    if (len <= 0) {
      break;
    } else {
      conn->buf_.offset_ += len;
    }
  }
  if (conn->readHandle_) {
    conn->readHandle_(conn->buf_.bufptr_, conn->buf_.offset_);
  }
  conn->buf_.offset_ = 0;
}

void ConnectionWriteCallback(struct bufferevent* buf, void* data) {
  auto conn = static_cast<Connection*>(data);
  if (conn->)
}

}  // namespace net

}  // namespace fver