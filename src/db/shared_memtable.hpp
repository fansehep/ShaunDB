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
#include "src/db/comp.hpp"
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

class Compactor;

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

  // 单个 memtable 的最大容量
  uint32_t maxMemTableSize_;

  // 等待 Compactor 的刷入
  std::shared_ptr<Compactor> compactor_;

  TaskWorker() : isRunning_(false) {}

  // 向任务队列里增加任务
  void addTask(Memtask hand);

  // 让 Memtable worker 继续运行
  void Notify();

  // 启动后台执行任务者.
  void Run();

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

  // memtable 的数量, 每个 memtable 的峰值容量
  void Init(const uint32_t memtable_N, const uint32_t singleMemTableSize);
  void Set(const std::shared_ptr<SetContext>& set_context);
  void Get(const std::shared_ptr<GetContext>& get_context);
  void Delete(const std::shared_ptr<DeleteContext>& del_context);


  //!!! 必须在 Init 之前调用, 2.
  void SetCompactorRef(const std::shared_ptr<Compactor>& compactor);

 private:
  // 每个 memTable 的最大容量
  uint32_t singleMemTableSize_;
  // memtableVec 的容量
  uint32_t memtable_N_;

  // 当请求来之后,
  std::vector<std::shared_ptr<TaskWorker>> taskworkers_;
  // 哈希算法
  // 将请求散列到不同的 memtable 中去
  std::hash<std::string_view> stdHash_;
  //
  std::shared_ptr<Compactor> comp_actor_;
};



}  // namespace db

}  // namespace fver

#endif