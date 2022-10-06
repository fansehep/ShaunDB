#ifndef SRC_BASE_BGWORKER_HPP_
#define SRC_BASE_BGWORKER_HPP_

#include "NonCopyable.hpp"
#include "Thread.hpp"
#include "ThreadSafeQueue.hpp"
#include "define.hpp"

namespace fver {

namespace base {

class BgWorker : public NonCopyable {
 public:
  using Task = std::function<void()>;

  BgWorker() : isStop_(false) {}

  void PushTask(const Task& task) { ThreadSimpleQueue_.Push(task); }

  void Start() {
    isStop_ = true;
    thread_.Start();
  }

 private:
  void ExecuteTask() {
    while (isStop_) {
      Task task;
      ThreadSimpleQueue_.WaitAndPop(&task);
    }
  }

  bool isStop_;
  fver::base::ThreadSafeQueue<Task> ThreadSimpleQueue_;
  fver::base::Thread thread_;
};

}  // namespace base

}  // namespace fver

#endif