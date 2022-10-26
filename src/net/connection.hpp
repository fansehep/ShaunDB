#ifndef SRC_NET_NET_CONNECTION_H_
#define SRC_NET_NET_CONNECTION_H_

extern "C" {
#include <assert.h>
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

#include "src/base/log/logbuffer.hpp"
#include "src/base/noncopyable.hpp"

using ::fver::base::NonCopyable;
using ::fver::base::log::Buffer;

namespace fver {
namespace net {
class NetServer;
class Connection;

// TODO: please use std::shared_ptr<Connection> to instead of Connection.
//  a easy way is create TCPConnectionImp
//  we can easy to do it!.
//  struct TCPConnectionImp {
//   std::shared_ptr<Connection> conn_;
//  }

static constexpr uint32_t kConnectionBufferSize = 4096;
void ConnectionWriteCallback(struct bufferevent* buf, void* data);
void ConnectionReadCallback(struct bufferevent* buf, void* data);
void ConnectionEventCallback(struct bufferevent* buf, short eventWhat,
                                    void* data);

using writeHandle = std::function<int(Connection*)>;
using closeHandle = std::function<int(Connection*)>;
using timeoutHandle = std::function<int(Connection*)>;
using readHandle = std::function<int(char*, size_t, Connection*)>;

// 使用智能指针管理所有连接
using ConnPtr = std::shared_ptr<Connection>;


class ConnImp;



class Connection : public NonCopyable {
 public:
  friend ConnImp;
  friend void ConnectionReadCallback(struct bufferevent* buf, void* data);
  friend void ConnectionWriteCallback(struct bufferevent* buf, void* data);
  friend void ConnectionEventCallback(struct bufferevent* buf, short eventWhat,
                                      void* data);
  Connection();
  ~Connection();
  bool Init();
  Connection(evutil_socket_t socket, readHandle rh, writeHandle wh,
             closeHandle ch, timeoutHandle th, NetServer* server);

  const std::string& getPeerIP() const { return peerIP_; }

  uint32_t getPeerPort() const { return peerPort_; }

  int Send(const char* msg, const size_t msg_size) {
    assert(nullptr != buf_);
    return bufferevent_write(buf_, msg, msg_size);
  }

 private:
  bool getPeerConnInfo();

  readHandle readHandle_;
  writeHandle writeHandle_;
  closeHandle closeHandle_;
  timeoutHandle timeoutHandle_;
  // 将每次读取到的数据存储在 Buffer 中,
  Buffer readBuf_;
  // 在 NetServer 中的fd
  evutil_socket_t socketFd_;
  // 单个 client 写入的buf
  struct bufferevent* buf_;
  // 单个 Connection 持有Server的引用
  NetServer* server_;
  // 时间戳
  struct timeval timeVal_;
  // 连接对等方的 IP
  std::string peerIP_;
  // 连接对等方的 port
  uint32_t peerPort_;
};



}  // namespace net

}  // namespace fver

#endif