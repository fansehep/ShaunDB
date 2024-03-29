#ifndef SRC_DB_COMPACTOR_H_
#define SRC_DB_COMPACTOR_H_

#include "absl/container/btree_map.h"

#include "src/base/log/logging.hpp"
#include "src/base/noncopyable.hpp"
#include "src/db/memtable.hpp"
#include "src/db/memtable_view_manager.hpp"
#include "src/db/shared_memtable.hpp"
#include "src/db/sstable_manager.hpp"
#include "src/util/iouring.hpp"
#include "src/db/dbconfig.hpp"

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
      |                     Compactor                   | --- : 每个 compworker 都拥有一个 io_uring 实例
      |-------------------------------------------------|
      |                 multi_thread_compact            |
      |-------------------------------------------------|
          ||                 ||                 ||
          ||                 ||                 ||
          ||                 ||                 ||
      |-------|         |---------|         |---------| -- Memtable_3_SSTable_0
      |-------|         |---------|         |---------| -- Memtable_3_SSTable_1
      |-------|         |---------|         |---------| -- Memtable_3_SSTable_2
          |                   |                  |
*/

// clang-format on

// 默认先使用 2 个线程进行 4 个 Memtable 的 Compaction
// TODO: 应该提供一个配置文件来进行设置
constexpr static uint32_t kDefaultCompactor_N = 2;

// io_uring 默认写入的队列深度
// 理论上来说, io_uring 写入的速度很快
// 而且这里的 compaction 每一次都是一个大 IO
constexpr static uint32_t kDefaultIOUringSize = 128;

// clang-format off
/*
 * comp_kv_data_str:
 * | 0 | index_1 | index_2 | ... | index_end |
 * 
 *
 *
 */

/*
 * comp_kv_meta_str:
 * | memtable_number             | maxkey_value_record | min_key_value_record | bloomfilter_data |
 * | 当前 sstable 所对应的 memtable| 最大
 *
 *
 */

// clang-format on

struct CompData {
  CompData() = default;
  // sync_data, 用来保存 io_uring 写入的数据
  // 当 iouring peek 成功返回之后释放
  std::shared_ptr<std::vector<char>> sync_data;
  // 记录当前数据 memtable 的编号
  uint32_t memtable_n;
  // 用来构造 memtable_view 所指向的 sstable_shared_ref
  std::shared_ptr<SSTable> sstable_ref;
  // CompData 有可能是 mino Compaction 或者 MajorCompaction.
  bool isMajorCompact = false;
  // 如果是 两层数据之间的 merge,
  std::pair<uint32_t, uint32_t> level_info;

  CompData(const std::shared_ptr<std::vector<char>>& data_ref,
           const uint32_t memtable_N, const std::shared_ptr<SSTable>& sstable)
      : sync_data(data_ref), memtable_n(memtable_N), sstable_ref(sstable) {}
};

// 对于每个 CompWorker
// major Compaction 将会由 Compactor 进行任务分发
struct CompWorker {
  // 每隔多少个前缀 kv 进行前缀压缩
  uint32_t prefix_size_;
  // 复用 memtable_kv 数据
  std::shared_ptr<std::vector<char>> comp_kv_data_str_;
  // 每次对于单个 memtable 的 kv index
  std::vector<uint32_t> comp_kv_index_vec_;
  //
  CompWorker() : isRunning_(false) {}
  // 是否正在运行
  bool isRunning_;

  // compWorker 持有 Compactor
  std::shared_ptr<Compactor> compactor_ref_;
  //
  std::thread bg_thread_;

  // 专门用来唤醒
  std::mutex notify_mtx_;
  std::condition_variable cond_;

  // io_uring 来做异步高性能写入, 读取还是做同步读
  util::iouring::IOUring iouring_;

  // TODO: 让每个 CompWorker 都拥有一个 IO_Uring实例
  // 由于在 Compactor 那里, 我们需要记录每次 compaction 的所对应的
  // memtable.

  // 用来保护 compaction 任务池
  std::mutex task_mtx_;

  // 快速交换, 防止阻塞
  std::vector<std::shared_ptr<Memtable>> bg_wait_to_sync_sstable_;

  std::vector<std::shared_ptr<Memtable>> wait_to_sync_sstable_;

  //
  //
  std::shared_ptr<SSTableManager> sstable_manager_;

  //
  //
  std::shared_ptr<SharedMemtable> shared_memtable_ref_;

  /*
   * memtable 层级view manager
   */
  std::shared_ptr<MemTableViewManager> memview_manager_;

  /*
   * 由于 io_uring 的写入是异步的, 所以我们必须保证在 io_uring写入之后
   * 再去释放需要写入的数据.
   */
  absl::btree_map<const char*, struct CompData> sync_data_map_;

  // 向单个 memtable 中增加 任务
  void AddSStable(const std::shared_ptr<Memtable>& memtable);
  // 启动 comp_worker, 等待  read_only table 刷入
  void Run();

  void SetCompactorRef(const std::shared_ptr<Compactor>&);

  std::string path;
  
  // 最大层数
  inline static uint32_t max_level_n_;
  inline static uint32_t memtable_N_;
  // 当前版本
  inline static uint32_t cur_version_;
};

// clang-format off
/*
 * memtable key_value style :
 *  | key_size | key_value | sequence_number | value_type | value_size | value_val |
 *  | 
 *
 * sstable key_value style :
 *  | 
 *
*/


/*
 * |       48 byte              |
 * |                            |
 * |   4 byte                |     4 byte             |        4 byte           |      4byte                     |
 * | max_key_value_index     | min_key_value_index    | bloom_filter_data_index | _key_value_size                |
 * | 当前 sstable key最大值    | 当前 sstable key最小值  |     布隆过滤器的数据下标    |      当前的 sstable 的key_value |
 * |   的下标                 |   的下标                |                         |              的容量             |
 * 
 *
 *   每隔 16 个key_value 将不采用前缀压缩
 * |      4 byte         |      4 byte         |   4 byte         |
 * | kv_data_index_0     |   kv_data_index_1   |  kv_data_index_2 |
 * |                     |                     |                  |
 *
 *
*/

/*
 * | shared_key_size | key_size | key_val | shared_value_size | value_size | value_val |
 * | var_int         |  ...
 * |
 * |
 * | key_size | key_val | value_size | value_val |
 *  .
 *  .
 */

/* |        Meta info                                   |              |
 * | create_time | level    |        memtable_number    | key_all_size | 
 * | uint64_t    | 当前的层级 | 对应的哪一个 memtable 的号码 |              |
 *
 *
 *
*/



// clang-format on
//
class Compactor : public NonCopyable {
 public:
  friend CompWorker;

  Compactor() = default;

  void Run();

  // 设置 memtable 的数量
  void Init(const DBConfig& db_config);

  // 向 Compactor 中增加只读 memtable, 等待被后台线程刷入
  void AddReadOnlyTable(const std::shared_ptr<Memtable>& mem_table);

  void SetCompWorkerCompactorRef(const std::shared_ptr<Compactor>& compactor);

  void SetSharedMemTableRef(
      const std::shared_ptr<SharedMemtable>& sharedmemtable);

  void SetSharedMemTableViewSSRef(
      const std::shared_ptr<MemTableViewManager>& manager);

 private:
  
  // memtable 的数量
  uint32_t memtable_n_;
  // 工作者的数量, 即有多少个 Compactor
  uint32_t task_workers_n_;
  // 最大层数
  // 当超过该层数, 触发 对应的levelmerge
  uint32_t max_level_n_;
  // 每个 memtable 对应一个 sstable
  // 后台负责将 readonly_memtable 刷入到磁盘中
  // number => vec<Memtable>
  std::mutex mtx_;
  //
  std::vector<std::shared_ptr<CompWorker>> bg_comp_workers_;

  // 保护 MemWorker 放置任务
  std::mutex memTaskmtx_;

  //
  std::shared_ptr<SSTableManager> sstable_manager_;
  
  // 用来记录 memtable 当前的层数
  // round_lobin 轮询方式放置任务
  // TODO: 考虑负载均衡
  // 但其实应该也不太顾虑, 因为每个 memtable 的大小应该相差不大
  uint32_t current_comp_index_;
  // sstable 存放的目录
  std::string db_path_;
  // 每隔多少个前缀 kv 进行前缀压缩
};

}  // namespace db

}  // namespace fver

#endif