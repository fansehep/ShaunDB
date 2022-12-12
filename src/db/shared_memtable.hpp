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
#include "src/db/memtable_view.hpp"
#include "src/db/memtable_view_manager.hpp"
#include "src/db/request.hpp"
#include "src/util/klrucache.hpp"
#include "src/db/dbconfig.hpp"

using ::fver::base::NonCopyable;
using ::fver::util::kLRUCache;

namespace fver {

namespace db {


// struct task {
//   std::optional<std::shared_ptr<SetContext>> set_context;
//   std::optional<std::shared_ptr<GetContext>> get_context;
//   std::optional<std::shared_ptr<DeleteContext>> del_context;
// };

class Compactor;

using Memtask =
    std::variant<std::shared_ptr<SetContext>,
                 std::shared_ptr<GetContext>,
                 std::shared_ptr<DeleteContext>,
                 std::shared_ptr<SnapShotContext>>;

class TaskWorker {
 public:
  //
  uint64_t remove_N_;
  // 单个 TaskWorker 保存上一次的 remove 的序列号.
  uint64_t preRemove_N_;
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

  // 将 memtbale_view 放置到该仓库中
  std::shared_ptr<MemTableViewManager> memview_manager_;

  std::mutex memtable_view_mtx_;

  // 在 Compactor 写入到文件之前
  // 我们可以获得两份数据的视图.
  // 第一份是 push 到 Compworker 中的 read_only memtable
  // 第二份是 Compworker 对于 read_only memtable 的序列化之后的数据
  //
  // read_only 的 memtable vec
  // 当 TaskWorker 当前的 memtable 到达阈值时
  // readonly_memtable_vec_ 会拥有一份引用
  // 当 CompWorker 完成minor_compaction 之后
  // 才会取消 readonly_memtable_vec_ 中的数据.
  std::vector<std::shared_ptr<Memtable>> readonly_memtable_vec_;
  //

  kLRUCache klrucache_;

  TaskWorker() : isRunning_(false) {}

  // 向任务队列里增加任务
  void addTask(Memtask hand);

  // 让 Memtable worker 继续运行
  void Notify();

  // 启动后台执行任务者.
  void Run();

  // 当前的 MemTaskWorker 的繁忙程度
  // TODO: is really thread safe?
  uint32_t getWorkerBusyN() {
    return handle_vec_.size() + bg_handle_vec_.size();
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
  void Init(const DBConfig& db_config);
  void Set(const std::shared_ptr<SetContext>& set_context);
  void Get(const std::shared_ptr<GetContext>& get_context);
  void Delete(const std::shared_ptr<DeleteContext>& del_context);
  void SnapShot(const std::shared_ptr<SnapShotContext>& snapshot_context);
  //!!! 必须在 Init 之前调用, 2.
  void SetCompactorRef(const std::shared_ptr<Compactor>& compactor);

  // 让单个 mem_worker 拥有 memtable_view_manager 的所有权.
  void SetMemTableViewRef(
      const std::shared_ptr<MemTableViewManager>& memtable_view_manager);


  auto getMemTaskWorkerLevel(const uint32_t n) {
    return taskworkers_[n]->memtable_->getCompactionN();
  }

  auto getMemTaskWorkerNumber(const uint32_t n) {
    return taskworkers_[n]->memtable_->getMemNumber();
  }

  /*
   * @n : 对应的 memtable 号码
   *
   */
  void PushRemoveReadonlyMemtableContext(const uint32_t n);

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