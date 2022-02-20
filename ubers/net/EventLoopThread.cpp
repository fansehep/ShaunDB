#include "EventLoopThread.h"
#include "EventLoop.h"
#include <assert.h>
#include <utility>

using namespace UBERS;
using namespace UBERS::net;

EventLoopThread::EventLoopThread(ThreadInitCallback func)
  : thread_([this]{ ThreadFunciton();}),
    mutex_(),
    cond_(),
    callback_(std::move(func))
{
}

EventLoopThread::~EventLoopThread()
{
  exiting_ = false;
  if(loop_ != nullptr)
  {
    loop_->Quit();
  }
}

EventLoop* EventLoopThread::StartLoop()
{
  assert(!thread_.started());
  thread_.start();
  {
    std::unique_lock<std::mutex> lock(mutex_);
    cond_.wait(lock, [this]{return loop_ != nullptr;});
  }
}

void EventLoopThread::ThreadFunciton()
{
  EventLoop loop;
  if(callback_)
  {
    callback_(&loop);
  }
  {
    std::unique_lock<std::mutex> lock(mutex_);
    loop_ = &loop;
    cond_.notify_one();
  }
  loop.Loop();
  std::scoped_lock<std::mutex> lock(mutex_);
  loop_ = nullptr;
}