#ifndef SRC_BASE_THREADPOOL_H_
#define SRC_BASE_THREADPOOL_H_

#include <functional>
#include <queue>
#include <vector>

#include "src/base/noncopyable.hpp"
#include "src/base/thread.hpp"
#include "src/base/threadsafequeue.hpp"

namespace fver::base {

class ThreadPool : public NonCopyable {
 public:
  using Task = std::function<void()>;
  explicit ThreadPool(int n) : n_(n), running_(false), que_() {}
  ~ThreadPool();
  void SetThreadPoolInitFunc(const Task& func) { Initfunc_ = func; }
  void Start();
  void AddTask(Task task) { que_.Push(task); }
  void Stop();

 private:
  int n_;
  void RunInThread();
  Task Initfunc_;
  bool running_;
  std::vector<std::unique_ptr<Thread>> workers_;
  ThreadSafeQueue<Task> que_;
};

}  // namespace fver::base

#endif