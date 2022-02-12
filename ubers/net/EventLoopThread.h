#ifndef _UBERS_EVENTLOOPTHREAD_H_
#define _UBERS_EVENTLOOPTHREAD_H_
#include <boost/noncopyable.hpp>
#include <mutex>
#include <condition_variable>
#include "../base/Thread.h"
namespace UBERS::net
{
class EventLoop;
class EventLoopThread : public boost::noncopyable
{
public:
  using ThreadInitCallback = std::function<void(EventLoop* )>;

  explicit EventLoopThread(ThreadInitCallback func = ThreadInitCallback());
  ~EventLoopThread();
  EventLoop* StartLoop();

private:
  void ThreadFunciton();

  EventLoop* loop_ = nullptr;
  bool exiting_ = false;
  Thread thread_;
  std::mutex mutex_;
  std::condition_variable cond_;
  ThreadInitCallback callback_;
};  
}
#endif