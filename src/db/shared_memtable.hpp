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
  SharedMemtable(int N = kDefaultSharedMemtableSize);
  void Put(std::shared_ptr<PutContext> put_context);
  void Get(std::shared_ptr<GetContext> get_context);
  void Delete(std::shared_ptr<DeleteContext> del_context);
 private:
  uint64_t seed_;
  int memtableN_;
  std::vector<Memtable> vec_memtable_;
};

}  // namespace db

}  // namespace fver

#endif