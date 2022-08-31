#ifndef SRC_BASE_THREADSAFEQUEUE_H_
#define SRC_BASE_THREADSAFEQUEUE_H_
#include <condition_variable>
#include <memory>
#include "NonCopyable.hpp"
#include <mutex>
namespace fver::base {
//* 线程安全的队列 用于线程池
template <typename type>
class ThreadSafeQueue : public NonCopyable {
 public:
  ThreadSafeQueue() : head_(new Node), tail_(head_.get()) {}

  void Push(type x) {
    std::shared_ptr<type> newval(std::make_shared<type>(std::move(x)));
    std::unique_ptr<Node> temp(new Node);
    {
      std::scoped_lock<std::mutex> lock(tailMutex_);
      tail_->val_ = newval;
      Node* const newTail = temp.get();
      tail_->next_ = std::move(temp);
      tail_ = newTail;
    }
    cv_.notify_one();
  }
  std::shared_ptr<type> WaitAndPop() {
    std::unique_ptr<Node> Old_head = WaitPopHead();
    return Old_head->val_;
  }

  void WaitAndPop(type& x) { std::unique_ptr<Node> Old_head = WaitPopHead(x); }

  std::shared_ptr<type> TryPop() {
    std::unique_ptr<Node> OldHead = TryPopHead();
    return OldHead ? OldHead->val_ : std::shared_ptr<type>();
  }

  bool TryPop(type& x) {
    std::unique_ptr<Node> Oldhead = TryPopHead();
    return Oldhead;
  }

  [[nodiscard]] bool empty() const {
    std::scoped_lock<std::mutex> lock(headMutex_);
    return head_.get() == GetTail();
  }

 private:
  //* 单个节点
  struct Node {
    std::shared_ptr<type> val_;
    std::unique_ptr<Node> next_;
  };

  Node* GetTail() {
    std::lock_guard<std::mutex> lock(tailMutex_);
    return tail_;
  }

  std::unique_ptr<Node> PopHead() {
    std::unique_ptr<Node> old_head = std::move(head_);
    head_ = std::move(old_head->next_);
    return old_head;
  }

  std::unique_lock<std::mutex> WaitForData() {
    std::unique_lock<std::mutex> lock(headMutex_);
    //* 头指针不等于尾指针，即有任务加入，wait成功
    cv_.wait(lock, [this] { return head_.get() != GetTail(); });
    return lock;
  }

  std::unique_ptr<Node> WaitPopHead() {
    std::unique_lock<std::mutex> lock(WaitForData());
    return PopHead();
  }

  std::unique_ptr<Node> WaitPopHead(type& x) {
    std::unique_lock<std::mutex> lock(WaitForData());
    x = std::move(*head_->val_);
    return PopHead();
  }

  std::unique_ptr<Node> TryPopHead() {
    std::scoped_lock<std::mutex> lock(headMutex_);
    if (head_.get() == GetTail()) {
      return std::unique_ptr<Node>();
    }
    return PopHead();
  }

  std::unique_lock<Node> TryPopHead(type& x) {
    std::lock_guard<std::mutex> lock(headMutex_);
    if (head_.get() == GetTail()) {
      return std::unique_ptr<Node>();
    }
    x = std::move(*head_->val_);
    return PopHead();
  }

  std::unique_ptr<Node> head_;
  Node* tail_;
  mutable std::mutex headMutex_;
  std::mutex tailMutex_;
  std::condition_variable cv_;
};
}  // namespace fver::base
#endif