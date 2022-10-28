#include "src/net/net_server.hpp"

#include "src/base/log/logging.hpp"
#include "src/net/connection.hpp"
#include "src/net/repeated_timer.hpp"

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
  maxConnCount_ = maxCount;
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
  auto ue = new_conn->Init();
  if (ue == false) {
    LOG_WARN("conn init fail");
  }
  // 使用 map 管理所有连接
  server->mtx_.lock();
  server->ConnectionMap_.insert({socket, new_conn});
  server->mtx_.unlock();
}

void NetServer::Run() { event_base_dispatch(eventBase_); }

bool NetServer::removeConn(evutil_socket_t fd) {
  mtx_.lock();
  auto iter = ConnectionMap_.find(fd);
  mtx_.unlock();
  if (iter == ConnectionMap_.end()) {
    LOG_WARN("No connection in map fd: {}", fd);
    return false;
  }
  LOG_TRACE("Conn ip: {} port: {} has be removed", iter->second->getPeerIP(),
            iter->second->getPeerPort());
  delete iter->second;
  mtx_.lock();
  ConnectionMap_.erase(fd);
  mtx_.unlock();
  return true;
}

Connection* NetServer::getConn(evutil_socket_t fd) {
  return ConnectionMap_[fd];
}

std::map<evutil_socket_t, Connection*>& NetServer::getConnMap() {
  return ConnectionMap_;
}

std::mutex* NetServer::getMutex() { return &mtx_; }


uint32_t NetServer::getConnCount() {
  return ConnectionMap_.size();
}

}  // namespace net

}  // namespace fver