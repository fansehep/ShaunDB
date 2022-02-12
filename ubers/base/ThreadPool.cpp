#include "ThreadPool.h"

using namespace UBERS;
using namespace UBERS::base;

ThreadPool::ThreadPool()
  : ThreadInitCallback_(),
    taskQueue_()
{
  if(this->running_)
  {
    Stop();
  }
}

void ThreadPool::Stop()
{
  running_ = false;
}

void ThreadPool::Start(int Threadnums)
{
  running_ = true;
  threads_.reserve(static_cast<unsigned long>(Threadnums));
  for(unsigned i = 0; i < Threadnums; ++i)
  {
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


