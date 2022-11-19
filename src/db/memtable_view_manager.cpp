#include "src/db/memtable_view_manager.hpp"

#include "src/db/key_format.hpp"
#include "src/db/memtable_view.hpp"
#include "src/db/status.hpp"

namespace fver {

namespace db {

void MemTableViewManager::Init(const uint32_t memtable_n) {
  this->memtable_n_ = memtable_n;
  mem_all_vec_.resize(memtable_n_);
  for (auto &iter : mem_all_vec_) {
    iter = std::make_unique<MemTableViewVec>();
  }
}

void MemTableViewManager::PushTableView(const uint32_t number, const char *data,
                                        const uint32_t data_size) {
  // btree_view_vec_[number].push_back(
  //     std::make_unique<MemTable_view>(data, data_size));
  assert(mem_all_vec_.size() > number);
  mem_all_vec_[number]->mtx_.lock();
  mem_all_vec_[number]->memtable_vec_.push_back(
      std::make_unique<MemTable_view>(data, data_size));
  mem_all_vec_[number]->mtx_.unlock();
}

void MemTableViewManager::getRequest(
    const uint32_t number, const std::shared_ptr<GetContext> &get_context) {
  mem_all_vec_[number]->mtx_.lock();
  for (auto r_iter = mem_all_vec_[number]->memtable_vec_.rbegin();
       r_iter != mem_all_vec_[number]->memtable_vec_.rend(); r_iter++) {
    (*r_iter)->Get(get_context);
    // 当前找到了
    if (get_context->code.getCode() == kOk) {
      return;
    // 从后往前遍历, 发现被删除了, 那么最新的 sstable 显示被删除
    // 直接返回即可.
    } else if (get_context->code.getCode() == kDelete) {
      return;
    }
  }
  mem_all_vec_[number]->mtx_.unlock();
}

}  // namespace db

}  // namespace fver