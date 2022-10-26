#include "src/net/net_server.hpp"

#include "src/net/connection.hpp"

namespace fver {

namespace net {

bool NetServer::Init(const uint32_t port, writeHandle wh, closeHandle ch,
                     timeoutHandle th, readHandle rh, uint32_t maxCount) {
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
  listener_ = evconnlistener_new_bind(eventBase_, ListenerCallback,
                                      static_cast<void*>(this),
                                      LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE,
                                      -1, (sockaddr*)(&sin_), sizeof(sin_));
  if (nullptr == listener_) {
    LOG_ERROR("listener create error!");
    return false;
  }
  port_ = port;
  return true;
}

void FverLogInit(int severity, const char* msg) {
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

// Listener 的回调函数
void ListenerCallback(struct evconnlistener* ev, evutil_socket_t socket,
                      struct sockaddr* addr, int socklen, void* arg) {
  NetServer* server = static_cast<NetServer*>(arg);
  if (server == nullptr) {
    LOG_ERROR("listen args server is nullptr!");
  }
  auto new_conn =
      new Connection(socket, server->read_handle_, server->write_handle_,
                     server->close_handle_, server->timeout_handle_, server);
  if (new_conn == nullptr) {
    LOG_ERROR("create connection error");
  }
  assert(true == new_conn->Init());
  // 使用 map 管理所有连接
  server->ConnectionMap_.insert({socket, new_conn});
}

bool NetServer::removeConn(evutil_socket_t fd) {
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

}  // namespace net

}  // namespace fver