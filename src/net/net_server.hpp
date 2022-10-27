#ifndef SRC_NET_SERVER_H_
#define SRC_NET_SERVER_H_

extern "C" {
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/event.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <sys/socket.h>
}

#include <map>
#include <memory>

#include "src/base/log/logging.hpp"
#include "src/base/noncopyable.hpp"
#include "src/base/threadsafequeue.hpp"
#include "src/net/connection.hpp"

using ::fver::base::NonCopyable;
using ::fver::base::ThreadSafeQueue;
using ::fver::base::log::Buffer;

namespace fver {
namespace net {

class RepeatedTimer;
class SingleTimer;

void FverLogInit(int serverity, const char* msg);
void ListenerCallback(struct evconnlistener* ev, evutil_socket_t socket,
                             struct sockaddr* addr, int socklen, void* arg);

// 默认支持最大连接是 1000 个连接
static constexpr uint32_t kMaxConnectionN = 1000;

class NetServer : public base::NonCopyable {
 public:
  friend Connection;
  friend ConnImp;

  friend void FverLogInit(int serverity, const char* msg);
  friend void ListenerCallback(struct evconnlistener* ev,
                               evutil_socket_t socket, struct sockaddr* addr,
                               int socklen, void* arg);

  NetServer() : eventBase_(nullptr), listener_(nullptr) {}
  ~NetServer() = default;
  bool Init(const uint32_t port, writeHandle wh, closeHandle ch,
            timeoutHandle th, readHandle rh,
            uint32_t maxCount = kMaxConnectionN);

  bool Run();

  Connection* getConn(evutil_socket_t fd) { return ConnectionMap_[fd]; }

  bool removeConn(evutil_socket_t fd);


  // 添加重复的定时器
  void AddRepeatedTimer(const RepeatedTimer& timer);

  // 添加一次性定时器
  void AddSingleTimer(const SingleTimer& timer);

 private:
  writeHandle write_handle_;
  closeHandle close_handle_;
  timeoutHandle timeout_handle_;
  readHandle read_handle_;

  uint32_t port_;
  uint32_t maxConnCount_;
  struct sockaddr_in sin_;
  struct event_base* eventBase_;
  struct evconnlistener* listener_;
  std::map<evutil_socket_t, Connection*> ConnectionMap_;
};

}  // namespace net

}  // namespace fver

#endif