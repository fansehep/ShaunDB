#include "src/db/sstable_manager.hpp"
#include "src/db/sstable.hpp"



namespace fver {

namespace db {



auto SSTableManager::getSSTable(const uint32_t number, const uint32_t level) {
  return sstable_vec_[number][level];
}

auto SSTableManager::getVecSize() {
  return sstable_vec_.size();
}

auto SSTableManager::getNumSize(const uint32_t number) {
  return sstable_vec_[number].size();
}


}



}