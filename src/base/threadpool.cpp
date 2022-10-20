#include "threadpool.hpp"
#include "src/base/threadpool.hpp"
#include <memory>

namespace fver::base {

void ThreadPool::Start() {
  running_ = true;
  workers_.reserve(static_cast<unsigned long>(n_));
  for (int i = 0; i < n_; i++) {
    workers_.emplace_back(std::make_unique<Thread>([this] { RunInThread(); }));
    workers_[i]->Start();
  }
  if (n_ == 0 && Initfunc_) {
    Initfunc_();
  }
}

void ThreadPool::Stop() {
  running_ = false;
  for (auto& thrd : workers_) {
    thrd->Join();
  }
}

void ThreadPool::RunInThread() {
  if (Initfunc_) {
    Initfunc_();
  }
  while (running_) {
    Task task;
    que_.WaitAndPop(task);
    if (task) {
      task();
    }
  }
}

ThreadPool::~ThreadPool() {
  if (running_) {
    Stop();
  }
}

};  // namespace fver::base