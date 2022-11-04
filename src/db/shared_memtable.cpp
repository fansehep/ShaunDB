#include "src/db/shared_memtable.hpp"

#include "src/base/log/logging.hpp"
#include "src/db/memtable.hpp"
#include "src/db/request.hpp"
#include "src/db/status.hpp"
#include "src/util/hash/xxhash64.hpp"

namespace fver {

namespace db {

void SharedMemtable::Init(uint32_t memtable_N) {
  assert(memtable_N != 0);
  memtable_N_ = memtable_N;
  memtableVec_.reserve(memtable_N);
  taskworkers_.reserve(memtable_N);
  for (auto& iter : memtableVec_) {
    iter = std::make_shared<Memtable>();
  }
  int i = 0;
  for (; i < memtable_N; i++) {
    taskworkers_[i] = std::make_shared<TaskWorker>();
    taskworkers_[i]->memtable_ = memtableVec_[i];
  }
}

void SharedMemtable::Run() {
  assert(memtable_N_ > 0);
  assert(memtableVec_.size() > 0);
  assert(taskworkers_.size() > 0);
  LOG_TRACE("shared_memtable start! memtable_n: {}", memtable_N_);
  for (auto iter : taskworkers_) {
    iter->Run();
  }
}

void SharedMemtable::Set(const std::shared_ptr<SetContext>& set_context) {
  auto idx = stdHash_(set_context->key) % memtable_N_;
  //
  Memtask task = set_context;
  taskworkers_[idx]->addTask(task);
  // should notify.
  taskworkers_[idx]->Notify();
}

void SharedMemtable::Get(const std::shared_ptr<GetContext>& get_context) {
  auto idx = stdHash_(get_context->key) % memtable_N_;
  Memtask task = get_context;
  taskworkers_[idx]->addTask(task);
  // should notify.
  taskworkers_[idx]->Notify();
}

void SharedMemtable::Delete(const std::shared_ptr<DeleteContext>& del_context) {
  auto idx = stdHash_(del_context->key) % memtable_N_;
  Memtask task = del_context;
  taskworkers_[idx]->addTask(task);
  // should notify.
  taskworkers_[idx]->Notify();
}

}  // namespace db

}  // namespace fver