#include <memory>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <boost/thread/latch.hpp>
#include "CurrentThread.h"
#include "Thread.h"
#include "Exception.h"
#include "Logging.h"

namespace UBERS
{
  namespace CurrentThread
  {
    thread_local int t_cachedTid = 0;
    thread_local std::string t_tidString;
  } // namespace CurrentThread

  namespace detail
  {
    //* std::function<void()>
    using ThreadFunc = UBERS::Thread::ThreadFunc;

    void RunInThread(const ThreadFunc& func, pid_t* tid, boost::latch* latch)
    {
      *tid = UBERS::CurrentThread::tid();
      latch->count_down();

      try
      {
        func();
      }
      catch(const Exception& ex)
      {
        fprintf(stderr, "exception caught in Thread %d \n", CurrentThread::tid());
        fprintf(stderr, "reason: %s \n", ex.what());
        fprintf(stderr, "stack trace: %s\n", ex.stackTrace());
        abort();
      }
      catch(const std::exception& ex)
      {
        fprintf(stderr, "exception caught in Thread %d \n", CurrentThread::tid());
        fprintf(stderr, "reason: %s\n", ex.what());
        abort();
      }
      catch(...)
      {
        fprintf(stderr, "unkonwn exception caught in Thread %d\n", CurrentThread::tid());
        throw;
      }
    }
  }
}
using namespace UBERS;

bool CurrentThread::isMainThread()
{
  return tid() == getpid();
}

Thread::Thread(ThreadFunc func)
  : func_(std::move(func))
{
}

void Thread::start()
{
  assert(!started_);
  started_ = true;
  thread_ = std::make_unique<std::thread>(std::thread(detail::RunInThread, func_, &tid_, &latch_));
  if(thread_ == nullptr)
  {
    started_ = false;
    LOG_OFF << "ThreadCreating is wrong!";
  }
  else
  {
    latch_.wait();
    assert(tid_ > 0);
  }
}
