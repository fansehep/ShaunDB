#ifndef SRC_DB_MEMTABLE_H_
#define SRC_DB_MEMTABLE_H_

#include <memory>
#include <cstdio>
#include <map>
#include "src/db/request.hpp"
#include "src/util/bloomfilter.hpp"

namespace fver {
namespace db {

class Memtable {
public:
  Memtable() = default;
  ~Memtable() = default;
  void Set(std::shared_ptr<SetContext> set_context);
  void Put(std::shared_ptr<PutContext> put_context);
  void Get(std::shared_ptr<GetContext> get_context);
  void Delete(std::shared_ptr<DeleteContext> del_context);
private:
  std::map<std::string, std::string> memMap_;
  util::BloomFilter<> bloomFilter_;
};

}

}


#endif