#ifndef SRC_DB_COMPACTOR_H_
#define SRC_DB_COMPACTOR_H_

#include "src/base/log/logging.hpp"
#include "src/base/noncopyable.hpp"
#include "src/db/memtable.hpp"
#include "src/db/shared_memtable.hpp"
#include "src/db/sstable_manager.hpp"

namespace fver {

namespace db {

/*
 当 Memtable 写到一定容量之后, 会变成 readonly
 Memtable 会使用一个新的表, 来进行写入, 但是
 Compactor 将会负责 将 readonly Memtable 写入到文件中
 单个 Memtable => SStable,
 Compaction 操作分为
  1. Minor_Compaction: 主动将 Memtable => SStable
  2. 当 SStable 到达一定层级时, 多个不同的 SStable 中间会有
     key 重叠, 为了减少这部分的写入放大. 需要将 多个 SStable
     合并成一个 SStable
*/

// clang-format off

/*
       Memtable_1         Memtable_2          Memtable_3
      |--------|          |--------|          |--------|
      |--------|          |--------|          |--------|
      |--------|          |--------|          |--------|
      |--------|          |--------|          |--------|
          ||                  ||                  ||
          ||                  ||                  ||
          ||                  ||                  ||
      |--------|          |--------|          |--------|
      |--------|          |--------|          |--------|
      |--------|          |--------|          |--------|
      |--------|          |--------|          |--------|
       read_only           read_only           read_only
          |                   |                   |
          |                   |                   |
          \                   |                   /
           \                  |                  /
      |-------------------------------------------------|
      |                     Compactor                   |
      |-------------------------------------------------|
      |                 multi_thread_compact            |
      |-------------------------------------------------|
          ||                 ||                 ||
          ||                 ||                 ||
          ||                 ||                 ||
      |-------|         |---------|         |---------| -- Memtable_3_SSTable_0
      |-------|         |---------|         |---------| -- Memtable_3_SSTable_1
      |-------|         |---------|         |---------| -- Memtable_3_SSTable_2
*/

// clang-format on

// 默认先使用 2 个线程进行 4 个 Memtable 的 Compaction
// TODO: 应该提供一个配置文件来进行设置
constexpr static uint32_t kDefaultCompactor_N = 2;

struct CompWorker {
  CompWorker() : isRunning_(false) {}
  bool isRunning_;
  std::thread bg_thread_;

  // 专门用来唤醒
  std::mutex notify_mtx_;
  std::condition_variable cond_;

  // 单个 CompWorker 持有  SStable 的所有权
  std::shared_ptr<SSTableManager> sstable_manager_;

  // 用来保护 compaction 任务池
  std::mutex task_mtx_;

  // 快速交换, 防止阻塞
  std::vector<std::shared_ptr<Memtable>> bg_wait_to_sync_sstable_;

  std::vector<std::shared_ptr<Memtable>> wait_to_sync_sstable_;

  // 向单个 memtable 中增加 任务
  void AddSStable(const std::shared_ptr<Memtable>& memtable);
  // 启动 comp_worker, 等待  read_only table 刷入
  void Run();
};

class Compactor : public NonCopyable {
 public:

  // 启动 Compactor. 
  void Run();

  // 设置 memtable 的数量
  void Init(const uint32_t size, const uint32_t worker_size,
            const std::string& sstable_path);

  // 向 Compactor 中增加只读 memtable, 等待被后台线程刷入
  void AddReadOnlyTable(const std::shared_ptr<Memtable>& mem_table);

 private:
  // memtable 的数量
  uint32_t memtable_n_;
  // 工作者的数量, 即有多少个 Compactor
  uint32_t task_workers_n_;
  // 每个 memtable 对应一个 sstable
  std::shared_ptr<SSTableManager> sstable_manager_;
  // 后台负责将 readonly_memtable 刷入到磁盘中
  std::vector<std::shared_ptr<CompWorker>> bg_comp_workers_;
  // round_lobin 轮询方式放置任务
  // TODO: 考虑负载均衡
  // 但其实应该也不太顾虑, 因为每个 memtable 的大小应该相差不大
  uint32_t current_comp_index_;
  // sstable 存放的目录
  std::string db_path_;
};

}  // namespace db

}  // namespace fver

#endif