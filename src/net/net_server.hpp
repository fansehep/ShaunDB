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
#include "src/net/net_connection.hpp"

using ::fver::base::NonCopyable;
using ::fver::base::ThreadSafeQueue;
using ::fver::base::log::Buffer;

namespace fver {
namespace net {

// 默认支持最大连接是 1000 个连接
static constexpr uint32_t kMaxConnectionN = 1000;

class NetServer : public base::NonCopyable {
 public:
  friend Connection;
  friend ConnImp;
  NetServer() : eventBase_(nullptr), listener_(nullptr) {}
  ~NetServer() = default;
  bool Init(const uint32_t port, writeHandle wh, closeHandle ch,
            timeoutHandle th, readHandle rh, uint32_t maxCount = kMaxConnectionN) {
    // 设置 Log 为 fver_log 格式函数
    event_set_log_callback(FverLogInit);
    eventBase_ = event_base_new();
    if (nullptr == eventBase_) {
      LOG_ERROR("server eventbase new error!");
      return false;
    }
    write_handle_ = wh;
    read_handle_ = rh;
    close_handle_ = ch;
    timeout_handle_ = th;
    memset(&sin_, 0, sizeof(sin_));
    sin_.sin_family = AF_INET;
    sin_.sin_port = htons(port);
    listener_ = evconnlistener_new_bind(
        eventBase_, ListenerCallback, static_cast<void*>(this),
        LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, -1, (sockaddr*)(&sin_),
        sizeof(sin_));
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
      LOG_INFO("NetServer start run listen port: {}", port_);
      return true;
    }
    return false;
  }

  // Listener 的回调函数
  static void ListenerCallback(struct evconnlistener* ev,
                               evutil_socket_t socket, struct sockaddr* addr,
                               int socklen, void* arg) {
    NetServer* server = static_cast<NetServer*>(arg);
    if (server == nullptr) {
      LOG_ERROR("listen args server is nullptr!");
    }
    auto conn =
        new Connection(socket, server->read_handle_, server->write_handle_,
                       server->close_handle_, server->timeout_handle_, server);
    if (conn == nullptr) {
      LOG_ERROR("create connection error");
    }
    assert(true == conn->Init());
    // 使用 map 管理所有连接
    server->ConnectionMap_.insert({socket, conn});
  }

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
        break;
      }
      default: {
        LOG_WARN("{}", msg);
      }
    }
  }

  Connection* getConn(evutil_socket_t fd) { return ConnectionMap_[fd]; }

  bool removeConn(evutil_socket_t fd) {
    auto iter = ConnectionMap_.find(fd);
    if (iter == ConnectionMap_.end()) {
      LOG_WARN("No connection in map fd: {}", fd);
      return false;
    }
    delete iter->second;
    ConnectionMap_.erase(fd);
    LOG_INFO("Connection fd: {} has be removed", fd);
    return true;
  }

 private:
  writeHandle write_handle_;
  closeHandle close_handle_;
  timeoutHandle timeout_handle_;
  readHandle read_handle_;

  uint32_t port_;
  struct sockaddr_in sin_;
  struct event_base* eventBase_;
  struct evconnlistener* listener_;
  std::map<evutil_socket_t, Connection*> ConnectionMap_;
};

}  // namespace net

}  // namespace fver

#endif