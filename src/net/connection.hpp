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

#include "src/base/noncopyable.hpp"
#include "src/net/conn_callback.hpp"

using ::fver::base::NonCopyable;

namespace fver {
namespace net {
class NetServer;
class Connection;
struct ConnImp;

// TODO: please use std::shared_ptr<Connection> to instead of Connection.
//  a easy way is create TCPConnectionImp
//  we can easy to do it!.
//  struct TCPConnectionImp {
//   std::shared_ptr<Connection> conn_;
//  }

class Connectioner;

using ConnPtr = std::shared_ptr<Connection>;

class Connection : public NonCopyable {
 public:
  // 每个 connection 默认拥有的 buf
  static constexpr uint32_t kConnectionBufferSize = 4096;
  // 连接状态
  enum ConnStat {
    kConnting,  // 正在连接连接
    kConnted,   // 已经建立连接
    kDisConn,   // 连接断开
  };
  friend struct ConnImp;
  friend Connectioner;
  friend void callback::ConnectionReadCallback(struct bufferevent* buf,
                                               void* data);
  friend void callback::ConnectionWriteCallback(struct bufferevent* buf,
                                                void* data);
  friend void callback::ConnectionEventCallback(struct bufferevent* buf,
                                                short eventWhat, void* data);
  Connection();
  ~Connection();
  bool Init();
  Connection(evutil_socket_t socket, readHandle rh, writeHandle wh,
             closeHandle ch, timeoutHandle th, NetServer* server);

  const std::string& getPeerIP() const { return peerIP_; }

  uint32_t getPeerPort() const { return peerPort_; }

  int Send(const char* msg, const size_t msg_size);

  struct ::bufferevent* getBufferEvent() { return buf_; }

  int getEvReadBufferLen() { return evbuffer_get_length(ev_read_buf_); }

  struct ::evbuffer* getReadBuf() { return ev_read_buf_; }

  int moveEvReadBuffer(char* data, const size_t data_len) {
    return evbuffer_remove(ev_read_buf_, data, data_len);
  }

  int moveEvWriteBuffer(char* data, const size_t data_len) {
    return evbuffer_remove(ev_write_buf_, data, data_len);
  }
  int copyEvReadBuffer(char* data, const size_t data_len) {
    return evbuffer_copyout(ev_read_buf_, data, data_len);
  }
  uint32_t getEvWriteBufferSize() { return evbuffer_get_length(ev_write_buf_); }
  int addToWriteBuffer(char* data, const size_t data_len) {
    return evbuffer_add(ev_write_buf_, data, data_len);
  }

  int addbufToWriteBuffer(struct ::evbuffer* buf) {
    return evbuffer_add_buffer(ev_write_buf_, buf);
  }

  void move_buf_read_to_write() {
    evbuffer_add_buffer(ev_write_buf_, ev_read_buf_);
  }

  int SendEvWriteBuffer();

  const auto& getConnText() {
    return conn_pair_;
  }

  void SetConnInithandle(connInitHandle conn_init_handle);

 private:
  bool getPeerConnInfo();

  // 可读事件触发的回调函数
  readHandle readHandle_;

  // 可写事件完成之后触发的回调函数
  writeHandle writeHandle_;

  // 连接关闭时触发的回调函数
  closeHandle closeHandle_;

  // 超时回调的函数
  timeoutHandle timeoutHandle_;

  // 连接建立的回调函数
  connInitHandle conninitHandle_;

  struct ::evbuffer* ev_read_buf_;
  struct ::evbuffer* ev_write_buf_;

  // 在 NetServer 中的fd
  evutil_socket_t socketFd_;
  // 单个 client 写入的buf
  struct bufferevent* buf_;
  // 单个 Connection 持有Server的引用
  NetServer* server_;
  // 时间戳, 当连接超时时 断开
  struct timeval timeVal_;
  // 连接对等方的 IP
  std::string peerIP_;
  // 连接对等方的 port
  uint32_t peerPort_;
  // 连接的状态
  ConnStat state_;

  // 用于参数传递
  std::pair<NetServer*, int> conn_pair_;
};

}  // namespace net

}  // namespace fver

#endif