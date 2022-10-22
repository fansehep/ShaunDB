#include "src/db/memtable.hpp"
#include "src/db/request.hpp"
#include "src/db/status.hpp"


namespace fver {

namespace db {

void Memtable::Put(std::shared_ptr<PutContext> put_context) {
  memMap_.insert(put_context->key, put_context->value);
  bloomFilter_.Insert(put_context->key);
  put_context->code.setCode(kOk);
  return;
}

void Memtable::Get(std::shared_ptr<GetContext> get_context) {
  if (false == bloomFilter_.IsMatch(get_context->key)) {
    get_context->code.setCode(kNotFound);
    return;
  }
  auto iter = memMap_.find(std::string(get_context->key));
  // 没有找到
  if (iter == memMap_.end()) {
    get_context->code.setCode(kNotFound);
  }
  return;
}


}


}