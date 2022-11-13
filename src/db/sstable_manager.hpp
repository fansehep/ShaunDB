#ifndef SRC_DB_SSTABLE_MANAGER_H_
#define SRC_DB_SSTABLE_MANAGER_H_
#include <vector>
#include "src/db/sstable.hpp"

#include <memory>


namespace fver {

namespace db {




class SSTableManager {
public:


  //
  auto getSSTable(const uint32_t number, const uint32_t level);

  // 返回指定 number 的层数.
  auto getNumSize(const uint32_t number);


  // 返回当前有多少个 vec_sstable
  auto getVecSize();


private:
  std::vector<std::vector<std::shared_ptr<SSTable>>> sstable_vec_;
};




}



}



#endif