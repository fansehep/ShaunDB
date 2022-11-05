#ifndef SRC_DB_SHARED_MEMTABLE_H_
#define SRC_DB_SHARED_MEMTABLE_H_

#include <condition_variable>
#include <memory>
#include <optional>
#include <string>
#include <thread>
#include <variant>
#include <vector>

#include "src/base/log/logging.hpp"
#include "src/base/noncopyable.hpp"
#include "src/db/memtable.hpp"
#include "src/db/request.hpp"

using ::fver::base::NonCopyable;

namespace fver {

namespace db {

// struct task {
//   std::optional<std::shared_ptr<SetContext>> set_context;
//   std::optional<std::shared_ptr<GetContext>> get_context;
//   std::optional<std::shared_ptr<DeleteContext>> del_context;
// };

using Memtask =
    std::variant<std::shared_ptr<SetContext>, std::shared_ptr<GetContext>,
                 std::shared_ptr<DeleteContext>>;

class TaskWorker {
 public:
  // TaskWorker 共享 std::shared_ptr<Memtable> 的所有权
  std::shared_ptr<Memtable> memtable_;

  // 保护 handle_vec_;
  std::mutex vec_mtx_;

  // 后台增加任务池, 真正执行的任务池
  std::vector<Memtask> handle_vec_;

  // 增加任务的任务池
  std::vector<Memtask> bg_handle_vec_;

  // 配合条件变量使用, 当有任务加入到TaskWorker 中,
  // 应该使用条件变量去唤醒
  std::mutex mtx_;

  // 条件变量用于唤醒 工作者
  std::condition_variable cond_;

  // 执行任务的工作者
  std::thread worker_;

  // 是否正在执行
  bool isRunning_;

  TaskWorker() : isRunning_(false) {}

  // 向任务队列里增加任务
  void addTask(Memtask hand) {
    vec_mtx_.lock();
    bg_handle_vec_.push_back(hand);
    vec_mtx_.unlock();
  }

  // 让 Memtable worker 继续运行
  void Notify() { cond_.notify_one(); }

  // 启动后台执行任务者.
  void Run() {
    isRunning_ = true;
    worker_ = std::thread([&]() {
      while (isRunning_) {
        // SharedMemtable 已经通知了,
        // 但可能此时还没有运行在这里
        // 需要判断一下
        if (bg_handle_vec_.empty()) {
          std::unique_lock<std::mutex> lgk(mtx_);
          cond_.wait(lgk);
        }
        // 快速交换, 以免引起阻塞.

        {
          vec_mtx_.lock();
          std::swap(bg_handle_vec_, handle_vec_);
          vec_mtx_.unlock();
        }
        LOG_INFO("memtable start execute");
        // TODO: 优化, 将 handle.index() 做成 宏
        for (auto& handle : handle_vec_) {
          // 糟糕的设计 ...
          // Set 请求
          if (handle.index() == 0) {

            auto set_context = std::get<std::shared_ptr<SetContext>>(handle);
            assert(set_context != nullptr);
            memtable_->Set(set_context);
            // Get 请求
          } else if (handle.index() == 1) {
            auto get_context = std::get<std::shared_ptr<GetContext>>(handle);
            assert(get_context != nullptr);
            memtable_->Get(get_context);
            // Delete 请求
          } else if (handle.index() == 2) {
            auto del_context = std::get<std::shared_ptr<DeleteContext>>(handle);
            memtable_->Delete(del_context);
          }
        }
        handle_vec_.clear();
      }
    });
  }

  // 停止线程运行
  void Stop() {
    isRunning_ = false;
    cond_.notify_one();
    if (worker_.joinable()) {
      worker_.join();
    }
  }
};

// 一组 SharedMemtable 拥有四个内存kv 表
static constexpr int kDefaultSharedMemtableSize = 4;

// TODO: 需要一种负载均衡的算法
//  默认使用 std_hash()

/*
 使用 hash 是正确的原因:
  1. 两次相同的请求肯定会散列到相同的 Memtable 中

*/

class SharedMemtable : public NonCopyable {
 public:
  SharedMemtable() = default;
  ~SharedMemtable();

  void Run();

  void Init(uint32_t memtable_N = kDefaultSharedMemtableSize);
  void Set(const std::shared_ptr<SetContext>& set_context);
  void Get(const std::shared_ptr<GetContext>& get_context);
  void Delete(const std::shared_ptr<DeleteContext>& del_context);

 private:
  // memtableVec 的容量
  uint32_t memtable_N_;
  // 一组 memtable, 本质上是 一组 B Tree
  std::vector<std::shared_ptr<Memtable>> memtableVec_;
  // 当请求来之后,
  std::vector<std::shared_ptr<TaskWorker>> taskworkers_;
  // 哈希算法
  // 将请求散列到不同的 memtable 中去
  std::hash<std::string_view> stdHash_;
};

}  // namespace db

}  // namespace fver

#endif