#ifndef _UBERS_NET_CHANNEL_H_
#define _UBERS_NET_CHANNEL_H_
#include <functional>
#include <memory>
#include <string>
#include <boost/noncopyable.hpp>

namespace UBERS::net
{
  class EventLoop;
  class TcpConnection;

  extern const unsigned kNoneEvent;
  extern const unsigned kReadEventLT;
  extern const unsigned kReadEventET;
  extern const unsigned kWriteEvent;

class Channel : public boost::noncopyable
{
public:
  using EventCallback = std::function<void()>;
  explicit Channel(EventLoop* loop, int fd);
  ~Channel();

  void HandleEvent();
  
  //* 设置几个回调函数
  void SetReadCallBack(EventCallback func);
  void SetWriteCallBack(EventCallback func);
  void SetCloseCallBack(EventCallback func);
  void SetErrorCallBack(EventCallback func);

  [[nodiscard]] int GetFd() const {return fd_;}
  [[nodiscard]] unsigned GetEvent() const { return this->events_;}

  void SetRevents(unsigned revents) { revents_ = revents;}
  void EnableEvents(unsigned events ) { events_ |= events; Update();}
  void DisableEvents(unsigned events) { events_ &= ~events; Update();}
  void DisableAll() { events_ = kNoneEvent; Update();}

  [[nodiscard]] bool IsNoneEvent() const { return events_ == kNoneEvent;}
  [[nodiscard]] bool IsWriting() const { return events_ & kWriteEvent;}
  [[nodiscard]] bool IsReading() const { return events_ & kReadEventLT;}

  [[nodiscard]] int status() const { return status_;}
  void SetStatus(int status) {this->status_ = status;}

  EventLoop* GetOwnerLoop() {return this->loop_;}
  void Remove();
  void ChannelReset(int SocketId);
private:
  void Update();

  //* Channel 所属的EventLoop
  EventLoop* loop_ = nullptr;
  //* 当前 Channel 所持有的文件描述符
  int fd_;
  //* 关注的事件
  unsigned events_ = 0;
  //* poll 实际返回的事件个数
  unsigned revents_ = 0;
  //* poll 通道状态
  int status_ = -1;
  //* 是否处于处理事件中
  bool eventHandling_ = false;
  bool addedToLoop_ = false;

  EventCallback readcallback_;
  EventCallback writecallback_;
  EventCallback closecallback_;
  EventCallback errorcallback_;
};

}
#endif