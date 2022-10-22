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

#include "src/net/net_connection.hpp"
#include "src/base/log/logging.hpp"
#include "src/base/noncopyable.hpp"

using ::fver::base::NonCopyable;

namespace fver {
namespace net {

class NetServer : public base::NonCopyable {
 public:
  NetServer() : eventBase_(nullptr), listener_(nullptr) {}
  ~NetServer();
  bool Init(const uint32_t port) {
    // 设置 Log 为 fver_log 格式函数
    event_set_log_callback(FverLogInit);
    eventBase_ = event_base_new();
    if (nullptr == eventBase_) {
      LOG_ERROR("server eventbase new error!");
    }
    memset(&sin_, 0, sizeof(sin_));
    sin_.sin_family = AF_INET;
    sin_.sin_port = htons(port);
    listener_ =
        evconnlistener_new_bind(eventBase_, ListenerCallback, this,
                                LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, 50,
                                (sockaddr*)(&sin_), sizeof(sin_));
    if (nullptr == listener_) {
      LOG_ERROR("listener create error!");
      return false;
    }
    port_ = port;
    return true;
  }

  bool Run() {
    auto re = event_base_dispatch(eventBase_);
    if (0 == re) {
      LOG_INFO("NetServer start run listen port: {}", port);
      return true;
    }
    return false;
  }

  static void ListenerCallback(struct evconnlistener* ev, evutil_socket_t s,
                               struct sockaddr* addr, int socklen, void* arg) {}
  static void FverLogInit(int severity, const char* msg) {
    switch (severity) {
      case _EVENT_LOG_DEBUG: {
        LOG_DEBUG("{}", msg);
        break;
      }
      case _EVENT_LOG_MSG: {
        LOG_INFO("{}", msg);
        break;
      }
      case _EVENT_LOG_WARN: {
        LOG_WARN("{}", msg);
        break;
      }
      case _EVENT_LOG_ERR: {
        LOG_ERROR("{}", msg);
      }
      default: {
        LOG_WARN("{}", msg);
      }
    }
  }

  std::shared_ptr<Connection> getConn(evutil_socket_t fd) {
    return ConnectionMap_[fd];
  }
 private:
  uint32_t port_;
  struct sockaddr_in sin_;
  struct event_base* eventBase_;
  struct evconnlistener* listener_;
  std::map<evutil_socket_t, std::shared_ptr<Connection>> ConnectionMap_;
};

}  // namespace net

}  // namespace fver

#endif