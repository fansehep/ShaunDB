#ifndef _UBERS_THREADPOOL_H_
#define _UBERS_THREADPOOL_H_
#include <boost/noncopyable.hpp>
#include <memory>
#include <utility>
#include <vector>
#include <functional>
#include "Thread.h"
#include "ThreadSafeQueue.hpp"

namespace UBERS::base
{
class ThreadPool : public boost::noncopyable
{
  public:
    using Task = std::function<void()>;
    explicit ThreadPool();
    ~ThreadPool();

    void SetThreadInitCallback(Task task) {this->ThreadInitCallback_ = task;}
    void Start(int Threadnums);
    void Stop();
    void AddTask(Task task) { taskQueue_.Push(std::move(task));}
  private:
    void RunInthread();
    Task ThreadInitCallback_;
    bool running_ = false;
    std::vector<std::unique_ptr<Thread>> threads_;
    ThreadSafeQueue<Task> taskQueue_;
};
}

#endif