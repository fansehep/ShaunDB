#include "src/db/shared_memtable.hpp"

#include <thread>

#include "src/base/log/logging.hpp"
#include "src/db/compactor.hpp"
#include "src/db/memtable.hpp"
#include "src/db/request.hpp"
#include "src/db/status.hpp"
#include "src/util/hash/xxhash64.hpp"

namespace fver {

namespace db {

void TaskWorker::addTask(Memtask hand) {
  vec_mtx_.lock();
  bg_handle_vec_.push_back(hand);
  vec_mtx_.unlock();
}

void TaskWorker::Notify() { cond_.notify_one(); }

uint32_t g_thread_n = 0;
std::string ThreadNpName = "memworker_";

void TaskWorker::Run() {
  isRunning_ = true;

  auto current_id = g_thread_n++;

  worker_ = std::thread([&]() {
    auto thread_name = ThreadNpName + std::to_string(current_id);
    auto ue = ::pthread_setname_np(::pthread_self(), thread_name.c_str());
    assert(ue != ERANGE);
    //
    while (isRunning_) {
      // SharedMemtable 已经通知了,
      // 但可能此时还没有运行在这里
      // 需要判断一下
      if (bg_handle_vec_.empty()) {
        std::unique_lock<std::mutex> lgk(mtx_);
        cond_.wait(lgk);
      }
      // 快速交换, 以免引起阻塞.

      {
        vec_mtx_.lock();
        std::swap(bg_handle_vec_, handle_vec_);
        vec_mtx_.unlock();
      }
      // TODO: 优化, 将 handle.index() 做成 宏
      for (auto& handle : handle_vec_) {
        // 糟糕的设计 ...
        // Set 请求
        if (handle.index() == 0) {
          auto set_context = std::get<std::shared_ptr<SetContext>>(handle);
          assert(set_context != nullptr);
          memtable_->Set(set_context);
          // Get 请求
        } else if (handle.index() == 1) {
          auto get_context = std::get<std::shared_ptr<GetContext>>(handle);
          assert(get_context != nullptr);
          memtable_->Get(get_context);
          // 触发回调函数
          if (get_context->get_callback) {
            get_context->get_callback(get_context);
          }
          // Delete 请求
        } else if (handle.index() == 2) {
          auto del_context = std::get<std::shared_ptr<DeleteContext>>(handle);
          //
          assert(del_context != nullptr);
          memtable_->Delete(del_context);
        }
      }

      handle_vec_.clear();
      // 当当前的写入超过 预期时, 将当前正在写入的表换下
      // 等待 compactor 刷入成为 sstable
      LOG_INFO("current_mem_size: {} maxMemtableSize: {}",
               memtable_->getMemSize(), maxMemTableSize_);
      if (memtable_->getMemSize() >= maxMemTableSize_) {
        memtable_->setReadOnly();
        // compactor_->AddReadOnlyTable(memtable_);
        // make_shared, 创建一个新的 memtable
        // 新创建的 memtable 应该继承原有的 memtable_number;
        auto origin_memtable_number = memtable_->getMemNumber();
        auto origin_compaction_n = memtable_->getCompactionN();
        //
        //
        assert(compactor_.get() != nullptr);
        compactor_->AddReadOnlyTable(memtable_);
        assert(memtable_.use_count() == 2);
        // 重新创建一个新的 Memtable
        // 释放所有权
        memtable_ = std::make_shared<Memtable>();
        assert(memtable_.use_count() == 1);
        // 设置 memtable 编号
        memtable_->setNumber(origin_memtable_number);
        memtable_->setCompactionN(++origin_compaction_n);
        LOG_TRACE("new memtable: {}", origin_memtable_number);
      }
    }
  });
}

void SharedMemtable::Init(const uint32_t memtable_N,
                          const uint32_t singletableSize) {
  assert(memtable_N != 0);
  memtable_N_ = memtable_N;
  taskworkers_.resize(memtable_N_);
  int i = 0;
  // 每个 memtable 的容量
  singleMemTableSize_ = singletableSize;
  for (; i < memtable_N_; i++) {
    taskworkers_[i] = std::make_shared<TaskWorker>();
    taskworkers_[i]->memtable_ = std::make_shared<Memtable>();
    // 编号
    taskworkers_[i]->memtable_->setNumber(i);
    taskworkers_[i]->maxMemTableSize_ = this->singleMemTableSize_;
    taskworkers_[i]->compactor_ = this->comp_actor_;
  }
}

void SharedMemtable::Run() {
  assert(memtable_N_ > 0);
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

SharedMemtable::~SharedMemtable() {
  for (auto& iter : taskworkers_) {
    iter->Notify();
    iter->Stop();
  }
}

void SharedMemtable::SetCompactorRef(
    const std::shared_ptr<Compactor>& compactor) {
  this->comp_actor_ = compactor;
  for (auto& iter : taskworkers_) {
    iter->compactor_ = compactor;
  }
}

void SharedMemtable::SetMemTableViewRef(
    const std::shared_ptr<MemTableViewManager>& memtable_view_manager) {
  assert(memtable_view_manager.get() != nullptr);
  for (auto& iter : taskworkers_) {
    iter->memview_manager_ = memtable_view_manager;
  }
}

}  // namespace db

}  // namespace fver