#ifndef SRC_DB_SHARED_MEMTABLE_H_
#define SRC_DB_SHARED_MEMTABLE_H_

#include <string>
#include <vector>

#include "src/base/noncopyable.hpp"
#include "src/db/memtable.hpp"
#include "src/db/request.hpp"


using ::fver::base::NonCopyable;

namespace fver {

namespace db {


static constexpr int kDefaultSharedMemtableSize = 16;


//TODO: 需要一种负载均衡的算法
// 默认使用 XXhash64()
class SharedMemtable : public NonCopyable {
 public:

 private:

};

}  // namespace db

}  // namespace fver

#endif