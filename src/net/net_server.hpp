#ifndef SRC_NET_SERVER_H_
#define SRC_NET_SERVER_H_

extern "C" {
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/event.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <sys/socket.h>
#include "event2/event_struct.h"
}

#include <map>
#include <memory>

#include "src/net/connection.hpp"

using ::fver::base::NonCopyable;

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
  friend RepeatedTimer;
  friend SingleTimer;
  friend void FverLogInit(int serverity, const char* msg);
  friend void ListenerCallback(struct evconnlistener* ev,
                               evutil_socket_t socket, struct sockaddr* addr,
                               int socklen, void* arg);

  NetServer() : eventBase_(nullptr), listener_(nullptr) {}
  ~NetServer() = default;
  bool Init(const uint32_t port, writeHandle wh, closeHandle ch,
            timeoutHandle th, readHandle rh,
            uint32_t maxCount = kMaxConnectionN);

  // default call event_base_dispatch
  // will block here
  // can let a thread run it, then a queue to save the info.
  void Run();

  Connection* getConn(evutil_socket_t fd);

  std::map<evutil_socket_t, Connection*>& getConnMap();

  bool removeConn(evutil_socket_t fd);

  std::mutex* getMutex();

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
  // 保护map
  std::mutex mtx_;
  std::map<evutil_socket_t, Connection*> ConnectionMap_;
};

}  // namespace net

}  // namespace fver

#endif