#include "src/db/memtable.hpp"

#include "src/db/request.hpp"
#include "src/db/status.hpp"

namespace fver {

namespace db {

void Memtable::Set(std::shared_ptr<SetContext> set_context) {
  memMap_.insert(std::pair{set_context->key, set_context->value});
  bloomFilter_.Insert(set_context->key);
  set_context->code.setCode(kOk);
  return;
}

// should be append the value
void Memtable::Put(std::shared_ptr<PutContext> put_context) {
  memMap_.insert(std::pair{put_context->key, put_context->value});
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

void Memtable::Delete(std::shared_ptr<DeleteContext> del_context) {
  if (false == bloomFilter_.IsMatch(del_context->key)) {
    del_context->code.setCode(kNotFound);
    return;
  }
  auto iter = memMap_.find(std::string(del_context->key));
  if (iter == memMap_.end()) {
    del_context->code.setCode(kNotFound);
  }
  return;
}

}  // namespace db

}  // namespace fver