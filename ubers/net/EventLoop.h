#ifndef _UBERS_EVENTLOOP_H_
#define _UBERS_EVENTLOOP_H_
#include <vector>
#include <memory>
#include <functional>
#include <atomic>
#include <boost/noncopyable.hpp>
#include <mutex>
#include "../base/TimeStamp.h"
#include "../base/Logging.h"
#include "../base/Thread.h"
#include "../base/CurrentThread.h"
#include "CallBack.h"

namespace UBERS::net
{
class Channel;
class Epoll;
class TimerManager;
class Timer;

class EventLoop : public boost::noncopyable
{
public:
  explicit EventLoop();
  ~EventLoop();

  void Loop();
  void Quit();

  void RunInLoop(const std::function<void()>& func);
  void QueueInLoop(const std::function<void()>& func);

  std::weak_ptr<Timer> RunAt(const TimeStamp time, std::function<void()> func);
  std::weak_ptr<Timer> RunEvery(const double interval, std::function<void()> func);
  std::weak_ptr<Timer> RunAfter(const double interval, std::function<void()> func);

  void WakeUp() const;
  void UpdateChannel(Channel* channel);
  void RemoveChannel(Channel* channel);

  void AssertInLoopThread()
  {
    if(!isInLoopThread())
    {
      AbortNotInLoopThread();
    }
  }

  bool isInLoopThread() const { return this->threadId_ == CurrentThread::tid();}
  bool EventHandling() const { return this->eventHandling_;}

private:
  void AbortNotInLoopThread();
  void HandleRead() const;
  void RunFunction();

  bool looping_ = false;
  std::atomic<bool> quit_;
  bool eventHandling_ = false;
  bool CallFunction_ = false;
  const pid_t threadId_;
  
  std::unique_ptr<Epoll> epoll_;
  std::unique_ptr<TimerManager> timermanager_;

  std::unique_ptr<Channel> wakeupChannel_;
  //* 唤醒当前线程
  int wakeupFd_;
  
  std::vector<Channel*> ActiveChannels_;
  mutable std::mutex mutex_;

  std::vector<std::function<void()>> functions_;
};
}
#endif
