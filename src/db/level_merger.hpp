#ifndef SRC_DB_LEVEL_MERGER_H_
#define SRC_DB_LEVEL_MERGER_H_

#include "src/db/memtable_view.hpp"

namespace fver {

namespace db {

//
bool MergeLevel(const MemTable_view& leftMview,
                const MemTable_view& rightMview,
                MemTable_view* result);



}

}  // namespace fver

#endif