#include "src/db/snapshot.hpp"

extern "C" {
#include <assert.h>
}

namespace fver {

namespace db {

void SnapShot::Init(const uint64_t number, const uint32_t memtable_n) {
  sequence_number_ = number;
  snapshot_data_.resize(memtable_n);
  assert(snapshot_data_.size() == memtable_n);
}

}  // namespace db

}  // namespace fver