#include "src/db/sstable_manager.hpp"

#include "src/db/sstable.hpp"

namespace fver {

namespace db {

auto SSTableManager::Init(const uint32_t memtable_n) {
  //
  memtable_n_ = memtable_n;
  sstable_vec_.resize(memtable_n);
  for (auto& iter : sstable_vec_) {
    iter.resize(12);
  }
}

auto SSTableManager::newSSTable(const uint32_t number, const uint32_t level)
    -> std::shared_ptr<SSTable> {
  sstable_vec_[number][level] = std::make_shared<SSTable>();
  return sstable_vec_[number][level];
}

auto SSTableManager::getSSTable(const uint32_t number, const uint32_t level) {
  return sstable_vec_[number][level];
}

auto SSTableManager::getVecSize() { return sstable_vec_.size(); }

auto SSTableManager::getNumSize(const uint32_t number) {
  return sstable_vec_[number].size();
}

}  // namespace db

}  // namespace fver