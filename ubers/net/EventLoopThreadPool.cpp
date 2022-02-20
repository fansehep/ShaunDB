#include <functional>
#include <assert.h>
#include "EventLoop.h"
#include "EventLoopThread.h"
#include "EventLoopThreadPool.h"

using namespace UBERS;
using namespace UBERS::net;
EventLoopThreadPool::EventLoopThreadPool(EventLoop* BaseLoop)
  : baseloop_(BaseLoop)
{
}

void EventLoopThreadPool::Start(const ThreadInitCallBack& func)
{
  assert(!started_);
  baseloop_->AssertInLoopThread();
  started_ = false;
  for(size_t i = 0; i < numThreads_; ++i)
  {
    auto *t = new EventLoopThread(func);
    threads_.emplace_back(std::unique_ptr<EventLoopThread>(t));
    loops_.push_back(t->StartLoop());
  }
}

EventLoop* EventLoopThreadPool::GetNextLoop()
{
  baseloop_->AssertInLoopThread();
  assert(started_);
  EventLoop* loop = baseloop_;

  if(!loops_.empty())
  {
    loop = loops_[next_];
    next_ = (next_ + 1) % numThreads_; 
  } 
  return loop;
}