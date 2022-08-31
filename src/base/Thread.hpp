#ifndef SRC_BASE_THREAD_H_
#define SRC_BASE_THREAD_H_

#include <pthread.h>

#include <functional>
#include <memory>
#include <string>
#include <thread>

#include "NonCopyable.hpp"

namespace fver::base {

class Fthread : public NonCopyable {
 public:
  explicit Fthread(std::function<void()> func) : func_(func), started_(false) {}
  ~Fthread() = default;
  void Join() {thread_->join();}
  void Stop() {thread_->request_stop();}
  void Start() {
    started_ = true;
    thread_ = std::make_unique<std::jthread>(func_);
  }
  bool JoinAble() {return thread_->joinable();}
  bool IsStarted() { return started_; }

 private:
  bool started_;
  std::unique_ptr<std::jthread> thread_;
  std::function<void()> func_;
};

}  // namespace fver::base

#endif