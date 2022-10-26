#include "src/db/exportdb.hpp"
#include "src/db/request.hpp"


namespace fver {

namespace db {

void DB::Put(std::shared_ptr<PutContext> put_context) {
  return shared_memtable_.Put(put_context);
}

void DB::Delete(std::shared_ptr<DeleteContext> del_context) {
  return shared_memtable_.Delete(del_context);
}

void DB::Get(std::shared_ptr<GetContext> get_context) {
  return shared_memtable_.Get(get_context);
}

void DB::Set(std::shared_ptr<SetContext> set_context) {
  return shared_memtable_.Set(set_context);
}

}



}