#ifndef SRC_BASE_CHANNEL_H_
#define SRC_BASE_CHANNEL_H_

#include <poll.h>

#include <functional>
#include <memory>

#include "src/base/NonCopyable.hpp"
#include "src/base/log/Logging.hpp"

namespace fver::net {
class EventLoop;
class Socket;
class Channel : public fver::base::NonCopyable {
 public:
  using EventCallback = std::function<void()>;
  void SetStartFunc(EventCallback func);
  void SetCloseFunc(EventCallback func);
  void SetReadFunc(EventCallback func);
  void SetWriteFunc(EventCallback func);
  void SetErrorFunc(EventCallback func);

  enum {
    kNoneEvent,
    kReadEvent,
    kWriteEvent,
  };

  void SetPollIndex(int idx) { pollindex_ = idx; }

  int GetPollIndex() { return pollindex_; }
  int GetFd() { return fd_; }
  void SetFd(int fd) { fd_ = fd; }
  void SetRevents(int eve) { revents_ = eve; }
  int GetRevents() { return revents_; }
  void SetEvents(int eve) { events_ = eve; }
  int GetEvents() { return events_; }
  bool IsNoneEvents() { return events_ == kNoneEvent; }
  void HandleEvents() {
    LOG_TRACE("HandleEvents");
    if ((revents_ & POLLHUP) && !(revents_ & POLLIN)) {
      if (CloseFunc_) {
        CloseFunc_();
      }
    }
    if (revents_ & (POLLERR | POLLNVAL)) {
      if (ErrorFunc_) {
        ErrorFunc_();
      }
    }
    if (revents_ & (POLLIN | POLLPRI | POLLRDHUP)) {
      if (ReadFunc_) {
        ReadFunc_();
      }
    }
    if (revents_ & POLLOUT) {
      if (WriteFunc_) {
        WriteFunc_();
      }
    }
  }

 private:
  int events_;
  int revents_;
  int fd_;
  // 连接建立起来的初始化函数
  EventCallback StartFunc_;
  // 连接 读取函数
  EventCallback WriteFunc_;
  // 连接 关闭的函数
  EventCallback CloseFunc_;
  // 连接 读取函数
  EventCallback ReadFunc_;
  // 连接 发生错误的函数
  EventCallback ErrorFunc_;
  // 在 poll / epoll 数组中的下标
  // 初始化 -1
  int pollindex_;
  EventLoop* loopptr_;
};

}  // namespace fver::net

#endif