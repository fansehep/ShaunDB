#include "src/db/memtable_view_manager.hpp"

#include "src/db/key_format.hpp"
#include "src/db/memtable_view.hpp"
#include "src/db/sstable.hpp"
#include "src/db/status.hpp"

namespace fver {

namespace db {

void MemTableViewManager::Init(const uint32_t memtable_n) {
  this->memtable_n_ = memtable_n;
  mem_all_vec_.resize(memtable_n_);
  for (auto &iter : mem_all_vec_) {
    iter = std::make_shared<MemTableViewVec>();
  }
}

uint32_t MemTableViewManager::getMemViewVecSize(const uint32_t number) {
  return mem_all_vec_[number]->memtable_vec_.size();
}

void MemTableViewManager::PushTableView(
    const uint32_t number, const char *data, const uint32_t data_size,
    const std::shared_ptr<SSTable> &sstable) {
  // btree_view_vec_[number].push_back(
  //     std::make_unique<MemTable_view>(data, data_size));
  assert(mem_all_vec_.size() > number);
  auto cur_memtable_vec_size = mem_all_vec_[number]->memtable_vec_.size();
  auto memtable_view = std::make_shared<MemTable_view>(
      data, data_size, number, cur_memtable_vec_size, sstable);
  mem_all_vec_[number]->mtx_.lock();
  mem_all_vec_[number]->memtable_vec_.push_back(memtable_view);
  mem_all_vec_[number]->mtx_.unlock();
}

void MemTableViewManager::PushTableMergeView(
    const uint32_t number, const char *data, const uint32_t data_size,
    const uint32_t level_1, const uint32_t level_2,
    const std::shared_ptr<SSTable> &sstable) {
  //
  assert(mem_all_vec_.size() > number);
  auto cur_memtable_merged_view = std::make_shared<MemTable_view>(
      data, data_size, number, level_1, level_2, sstable);
  assert(level_1 < mem_all_vec_[number]->memtable_vec_.size());
  assert(level_2 < mem_all_vec_[number]->memtable_vec_.size());
  mem_all_vec_[number]->mtx_.lock();
  //
  // 锁住, 将 level_1 和 level_2 之间的memtable_view 释放掉.
  mem_all_vec_[number]->memtable_vec_.erase(
      mem_all_vec_[number]->memtable_vec_.begin() + level_1);
  mem_all_vec_[number]->memtable_vec_.erase(
    mem_all_vec_[number]->memtable_vec_.begin() + level_2);
  //TODO: should change the merged info?
  mem_all_vec_[number]->mtx_.unlock();
}

void MemTableViewManager::getRequest(
    const uint32_t number, const std::shared_ptr<GetContext> &get_context) {
  mem_all_vec_[number]->mtx_.lock();
  for (auto r_iter = mem_all_vec_[number]->memtable_vec_.rbegin();
       r_iter != mem_all_vec_[number]->memtable_vec_.rend(); r_iter++) {
    (*r_iter)->Get(get_context);
    // 当前找到了
    if (get_context->code.getCode() == StatusCode::kOk ||
        get_context->code.getCode() == StatusCode::kDelete) {
      return;
    }
  }
  mem_all_vec_[number]->mtx_.unlock();
}

}  // namespace db

}  // namespace fver