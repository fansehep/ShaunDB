#include "ThreadPool.h"
#include <stdio.h>
#include "Exception.h"
using namespace UBERS;
using namespace UBERS::base;

ThreadPool::ThreadPool()
  : ThreadInitCallback_(),
    taskQueue_(),
    running_(false)
{

}
ThreadPool::~ThreadPool()
{
  if(running_)
  {
    Stop();
  }
}

//* FIXME
void ThreadPool::Stop()
{
  running_ = false;
  for(auto& thd : threads_)
  {
    thd->Join();
  }
}


void ThreadPool::Start(int Threadnums)
{
  running_ = true;
  threads_.reserve(static_cast<unsigned long>(Threadnums));
  for(int i = 0; i < Threadnums; ++i)
  {
    //* 每个线程都执行  RunInthread() 
    threads_.emplace_back(std::make_unique<Thread>([this]{ RunInthread();}));
    threads_[i]->start();
  }
  if(Threadnums == 0 && ThreadInitCallback_)
  {
    ThreadInitCallback_();
  }
}

void ThreadPool::RunInthread()
{
  try
  {
    if(ThreadInitCallback_)
    {
      ThreadInitCallback_();
    }
    while(running_)
    {
      Task task;
      taskQueue_.WaitAndPop(task);
      if(task)
      {
        task();
      }
    }
  }
  catch(const Exception& ex)
  {
    fprintf(stderr, "exception caught in ThreadPool \n");
    fprintf(stderr, "reason: %s\n", ex.what());
    fprintf(stderr, "stack trace: %s\n", ex.stackTrace());
    abort();
  }
  catch(const std::exception& ex)
  {
    fprintf(stderr, "exception caught in ThreadPool");
    fprintf(stderr, "reason: %s\n", ex.what());
    abort();
  }
  catch(...)
  {
    fprintf(stderr, "unknown exception caught in ThreadPool ");
    throw; // rethrow
  }
}


