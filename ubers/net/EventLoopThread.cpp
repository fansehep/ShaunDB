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
    ;
  }
}
