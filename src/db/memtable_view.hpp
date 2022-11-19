#ifndef SRC_DB_MEMTABLE_VIEW_H_
#define SRC_DB_MEMTABLE_VIEW_H_



#include "src/db/key_format.hpp"
#include "src/db/request.hpp"
#include "src/util/bloomfilter.hpp"


namespace fver {

namespace db {






// readonly
class MemTable_view {
public:
  //
  MemTable_view() : isReadable_(false) {}
  ~MemTable_view() = default;
  //

  MemTable_view(const char* data, const uint32_t data_size);

  bool Init(std::string_view mmap_view);


  void Reset(const char* data, const uint32_t data_size);

  void Get(const std::shared_ptr<GetContext>& get_context);

  bool isReadAble() {
    return isReadable_;
  }

  // 当前 memTable_view 只读
  void setReadOnly() {
    isReadable_ = true;
  }

private:

  // 当前是否可读.
  bool isReadable_;


  // std::set<std::string_view>
  MemBTreeView memMapView_;

  std::unique_ptr<util::BloomFilter<>> bloomFilter_;
};




}



}


#endif