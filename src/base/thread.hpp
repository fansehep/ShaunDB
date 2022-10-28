#ifndef SRC_BASE_THREAD_H_
#define SRC_BASE_THREAD_H_

#include <pthread.h>

#include <functional>
#include <memory>
#include <string>
#include <thread>

#include "src/base/noncopyable.hpp"

namespace fver {
namespace base {


class Thread : public NonCopyable {
 public:
  explicit Thread(std::function<void()> func) : func_(func), started_(false) {}
  ~Thread() = default;
  void Join() { thread_->join(); }
  void Stop() { thread_->request_stop(); }
  void Start() {
    started_ = true;
    thread_ = std::make_unique<std::jthread>(func_);
  }
  bool JoinAble() { return thread_->joinable(); }
  bool IsStarted() { return started_; }

 private:
  std::unique_ptr<std::jthread> thread_;
  std::function<void()> func_;
  bool started_;
};

}  // namespace base
}  // namespace fver

#endif