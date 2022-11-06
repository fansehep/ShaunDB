#include "src/db/sstable_manager.hpp"

namespace fver {

namespace db {

/*
--SSLevel_0 -- -- memtable_0_sstable_0.sst
               -- memtable_0_sstable_1.sst
               -- memtable_0_sstable_2.sst
               -- memtable_0_sstable_3.sst
--SSLevel_1 -- -- memtable_1_sstable_0.sst
               -- memtable_1_sstable_1.sst
               -- memtable_1_sstable_2.sst
               -- memtable_1_sstable_3.sst
--SSLevel_2 -- -- memtable_2_sstable_0.sst
*/


void SSTableManager::Init(const uint32_t MemTableN,
                          const std::string& sstable_path,
                          const uint32_t level_size) {
  current_memtable_n_ = MemTableN;
  
}

}  // namespace db

}  // namespace fver