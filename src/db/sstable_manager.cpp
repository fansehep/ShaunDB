#include "src/db/sstable_manager.hpp"

#include "src/db/dbconfig.hpp"
#include "src/db/sstable.hpp"

namespace fver {

namespace db {

const uint32_t kDefaultSSTableSize = 12;

void SSTableManager::Init(const DBConfig& db_config) {
  memtable_n_ = db_config.memtable_N;
  sstable_vec_.resize(memtable_n_);
  for (auto& iter : sstable_vec_) {
    iter.reserve(kDefaultSSTableSize);
  }
  db_path_ = db_config.db_path;
}

auto SSTableManager::newSSTable(const uint32_t number, const uint32_t level)
    -> std::shared_ptr<SSTable> {
  sstable_vec_[number].push_back(std::make_shared<SSTable>());
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