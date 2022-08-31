#ifndef SRC_BASE_CHANNEL_H_
#define SRC_BASE_CHANNEL_H_

#include "src/base/NonCopyable.hpp"
#include <functional>
#include <memory>



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

private:
  enum {
    kNoneEvent,
    kReadEvent,
    kWriteEvent,
  };
  Socket fd_;
  // 连接建立起来的回调函数
  EventCallback StartFunc_;
  // 连接 读取函数
  EventCallback WriteFunc_;
  // 连接 关闭的函数
  EventCallback CloseFunc_;
  // 连接 读取函数
  EventCallback ReadFunc_;
  // 连接 发生错误的函数
  EventCallback ErrorFunc_;
  EventLoop* loopptr_;
};



}

#endif