#ifndef SRC_DB_MEMTABLE_VIEW_MANAGER_H_
#define SRC_DB_MEMTABLE_VIEW_MANAGER_H_

#include "src/db/key_format.hpp"
#include "src/db/memtable_view.hpp"
#include "src/db/request.hpp"

namespace fver {

namespace db {

struct MemTableViewVec {
  std::vector<std::shared_ptr<MemTable_view>> memtable_vec_;
  // std::mutex 只会在 push and get 之间上锁
  // 当需要改变 MemTable_view 的时候, 还是由单一的 MemTaskWorker 进行改变
  std::mutex mtx_;
  // 是否处于被Compaction 过程中
  std::atomic<bool> isCompaction_;
  // 尝试获得 MemTableViewVec 的锁
  bool getCompactLock() {
    bool expectedVal = false;
    return isCompaction_.compare_exchange_strong(expectedVal, true);
  }
  // 完成 Compaction 后标记当前level已经完成
  void finishCompact() { isCompaction_ = true; }
};

class SSTable;

class MemTableViewManager {
 public:
  MemTableViewManager() = default;
  ~MemTableViewManager() = default;

  void Init(const uint32_t memtable_n);

  /**
   * @number: 需要 push 的序列号
   * @data: mmap 文件起始地址
   * @data_size:
   * 当 compactor 完成 read_only_memtable => memtable 之后
   * 将视图放入这里.
   */
  void PushTableView(const uint32_t number, const char* data,
                     const uint32_t data_size,
                     const std::shared_ptr<SSTable>& sstable);

  /**
   * @number: memtable的编号
   * @data: 
   * @data_size
   * 当 compactor 完成任意两层之间的 level 之间的合并时,
   * 都需要将合并之后的数据 Push进来.
   */
  void PushTableMergeView(const uint32_t number,
                          const char* data,
                          const uint32_t data_size,
                          const uint32_t level_1,
                          const uint32_t level_2,
                          const std::shared_ptr<SSTable>& sstable);

  /**
   * @number: 内存表的序列号
   * @get_context: 获取请求
   */
  void getRequest(const uint32_t number,
                  const std::shared_ptr<GetContext>& get_context);

  // 返回 编号为 number 的 memtable_vec的容量
  uint32_t getMemViewVecSize(const uint32_t number);

  std::shared_ptr<MemTableViewVec> getMemNVec(const uint32_t n) {
    return mem_all_vec_[n];
  }

 private:
  // 内存表的数量
  uint32_t memtable_n_;

  // 层级的 memtable_view 视图
  std::vector<std::shared_ptr<MemTableViewVec>> mem_all_vec_;
};

}  // namespace db

}  // namespace fver

#endif