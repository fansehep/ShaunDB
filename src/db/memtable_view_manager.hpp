#ifndef SRC_DB_MEMTABLE_VIEW_MANAGER_H_
#define SRC_DB_MEMTABLE_VIEW_MANAGER_H_

#include "src/db/key_format.hpp"
#include "src/db/memtable_view.hpp"
#include "src/db/request.hpp"

namespace fver {

namespace db {

struct MemTableViewVec {
  std::vector<std::unique_ptr<MemTable_view>> memtable_vec_;
  std::mutex mtx_;
};

class MemTableViewManager {
 public:
  MemTableViewManager() = default;
  ~MemTableViewManager() = default;

  void Init(const uint32_t memtable_n);

  /*
   @number: 需要 push 的序列号
   @data: mmap 文件起始地址
   @data_size:
  */
  void PushTableView(const uint32_t number, const char* data,
                     const uint32_t data_size);

  /*
   * @number: 内存表的序列号
   * @get_context: 获取请求 
  */
  void getRequest(const uint32_t number,
                  const std::shared_ptr<GetContext>& get_context);

  // 返回 编号为 number 的 memtable_vec的容量
  uint32_t getMemViewVecSize(const uint32_t number);

 private:
  // 内存表的数量
  uint32_t memtable_n_;

  // 层级的 memtable_view 视图
  std::vector<std::unique_ptr<MemTableViewVec>> mem_all_vec_;
};

}  // namespace db

}  // namespace fver

#endif