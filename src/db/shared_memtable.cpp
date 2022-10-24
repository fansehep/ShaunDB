#include "src/db/shared_memtable.hpp"

#include "src/base/log/logging.hpp"
#include "src/db/request.hpp"
#include "src/db/status.hpp"
#include "src/util/hash/xxhash64.hpp"

namespace fver {

namespace db {

SharedMemtable::SharedMemtable(int N) : memtableN_(N) {
  seed_ = (static_cast<uint64_t>(rand()) << 32) | static_cast<uint64_t>(rand());
  vec_memtable_.reserve(N);
}

void SharedMemtable::Put(std::shared_ptr<PutContext> put_context) {
  auto index =
      XXHash64::hash(put_context->key.data(), put_context->key.size(), seed_) %
      memtableN_;
  vec_memtable_[index].Put(put_context);
  LOG_INFO("put memtable: {} key: {} value: {}", index, put_context->key,
           put_context->value);
}

void SharedMemtable::Get(std::shared_ptr<GetContext> get_context) {
  auto index =
      XXHash64::hash(get_context->key.data(), get_context->key.size(), seed_) %
      memtableN_;
  vec_memtable_[index].Get(get_context);
  if (get_context->code.getCode() == kOk) {
    LOG_INFO("get memtable: {} key: {} value: {} ok", index, get_context->key,
             get_context->value);
  } else {
    LOG_WARN("get memtable: {} key: {} error: {}", index, get_context->key,
             get_context->code.getCodeStr());
  }
}

void SharedMemtable::Delete(std::shared_ptr<DeleteContext> del_context) {
  auto index =
      XXHash64::hash(del_context->key.data(), del_context->key.size(), seed_) %
      memtableN_;
  vec_memtable_[index].Delete(del_context);
  if (del_context->code.getCode() == kOk) {
    LOG_INFO("del memtable: {} key: {} ok", index, del_context->key);
  } else {
    LOG_WARN("del memtable: {} key: {} fail {}", index, del_context->key,
             del_context->code.getCodeStr());
  }
}

}  // namespace db

}  // namespace fver