#ifndef SRC_NET_NET_CONNECTION_H_
#define SRC_NET_NET_CONNECTION_H_

extern "C" {
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/event.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <sys/socket.h>
}

#include <functional>
#include <map>
#include <memory>

#include "src/base/log/logging.hpp"
#include "src/base/noncopyable.hpp"

using ::fver::base::log::Buffer;

namespace fver {
namespace net {

static constexpr uint32_t kConnectionBufferSize = 4096;
static void ConnectionWriteCallback(struct bufferevent* buf, void* data);
static void ConnectionReadCallback(struct bufferevent* buf, void* data);
static void ConnectionEventCallback(struct bufferevent* buf, void* data);

class Connection {
 public:
  friend void ConnectionReadCallback(struct bufferevent* buf, void* data);
  Connection() : buf_(kConnectionBufferSize) {}

 private:
  std::function<void (char*, size_t)> readHandle_;
  
  // 在 NetServer 中的fd
  evutil_socket_t socketFd_;
  // 单个 client 写入的buf
  Buffer buf_;
};

}  // namespace net

}  // namespace fver

#endif