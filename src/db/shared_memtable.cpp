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

// for test the MemTable_view is right?
#ifdef DB_DEBUG

void TaskWorker::TestMemTableView(
    const std::shared_ptr<MemTable_view>& memtable_view,
    const std::shared_ptr<Memtable>& memtable) {
  // 1. 测试 bloom_filter 是否正确被格式化
  std::string_view memtable_bloomfilter_view(
      memtable_view->getBloomFilterPtr(), memtable_view->getBloomFilterSize());
  std::string_view memtable_bloomfilter(memtable->getFilterData()->getData(),
                                        memtable->getFilterData()->getSize());
  if (memtable_bloomfilter_view != memtable_bloomfilter) {
    LOG_ERROR("format error!");
    assert(false);
  }
  // 2. 验证 memtable_view 和 memtable 的正确性
}

#endif

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
#ifdef DB_DEBUG
          test_Map_[set_context->key] = set_context->value;
#endif
          memtable_->Set(set_context);
          LOG_INFO("memtable set key: {} value: {} ok mem_size: {}",
                   set_context->key, set_context->value,
                   memtable_->getMemSize());
          // Get 请求
        } else if (handle.index() == 1) {
          auto get_context = std::get<std::shared_ptr<GetContext>>(handle);
          assert(get_context != nullptr);
          memtable_->Get(get_context);
          // 如果在当前内存表中直接找到
          // 立即触发回调函数返回即可.
          // 在内存表已经找到或者发现被删除
          if (get_context->code.getCode() == StatusCode::kOk ||
              get_context->code.getCode() == StatusCode::kDelete) {
            // 触发回调函数
            if (get_context->get_callback) {
              get_context->get_callback(get_context);
            }
          }
          // 没有在当前内存表中找到
          if (get_context->code.getCode() == StatusCode::kNotFound) {
            // 从只读内存表中找
            for (auto riter = readonly_memtable_vec_.rbegin();
                 riter != readonly_memtable_vec_.rend(); riter++) {
              (*riter)->Get(get_context);
              // 在最新中的只读内存表中, 如果找到
              // 或者被删除, 都即可返回.
              // 找到了才触发回调函数
              if (get_context->code.getCode() == StatusCode::kOk ||
                  get_context->code.getCode() == StatusCode::kDelete) {
                if (get_context->get_callback) {
                  get_context->get_callback(get_context);
                }
                break;
              }
            }
          }
          // 只读 memtable 和 当前内存表中都没有找到
          // 在 memtable_view_vec 中找
          if (get_context->code.getCode() == StatusCode::kNotFound) {
            memview_manager_->getRequest(memtable_->getMemNumber(),
                                         get_context);
            // 直接触发回调函数.
            if (get_context->get_callback) {
              get_context->get_callback(get_context);
            }
          }
          // Delete 请求
        } else if (handle.index() == 2) {
          auto del_context = std::get<std::shared_ptr<DeleteContext>>(handle);
          //
          assert(del_context != nullptr);
          // TODO
          // 当无法在 memtable 中找到需要被删除的
          memtable_->Delete(del_context);
          // 所需要删除 kv 记录就在当前内存表中
          if (del_context->code.getCode() == StatusCode::kOk) {
            // 直接触发回调函数即可
            if (del_context) {
              del_context->del_callback(del_context);
            }
          } else if (del_context->code.getCode() == StatusCode::kNotFound) {
            // TODO should be view
            auto get_for_del_context = std::make_shared<GetContext>();
            /*
             * 当前是一个只读的 readonly_memtable_vec
             *
             *
             */
            get_for_del_context->key = del_context->key;
            for (auto r_iter = readonly_memtable_vec_.rbegin();
                 r_iter != readonly_memtable_vec_.rend(); r_iter++) {
              (*r_iter)->Get(get_for_del_context);
              (*r_iter)->Get(get_for_del_context);
              // 如果在 read_only_memtable_vec 中找到
              // 那么我们此时只能在当前的 memtable 中新加入一条删除记录.
              if (get_for_del_context->code.getCode() == StatusCode::kOk) {
                memtable_->InsertDeleteRecord(del_context);
                // 在 read_only_memtable_vec 中发现该条记录已经被删除
                // 那么直接返回即可
              } else if (get_for_del_context->code.getCode() ==
                         StatusCode::kDelete) {
                // 当前所需要删除的 kv 记录已经被删除
                del_context->code.setCode(StatusCode::kDelete);
                if (del_context->del_callback) {
                  del_context->del_callback(del_context);
                }
              }
            }
          }
          // erase the read_only memtable_vec head
#ifdef DB_DEBUG
          auto test_iter = test_Map_.find(del_context->key);
          if (test_iter == test_Map_.end()) {
          }
#endif
          // TODO: 事实上, 我们并不需要让 RemoveReadOnlyMemTableContext 成为
          // 这里的 MemTask 的一员, 因为 MemTask 只会由主线程分发,
          // remove...context 将会由 Compactor 线程分发 告诉当前的 worker
          // 可以触发 read_only_memtable 析构
        } else if (handle.index() == 3) {
          // 释放已经变成 memtable_view 的 memtable
          // TODO: erase the vec.begin() performance ???
          // but
          if (false == readonly_memtable_vec_.empty()) {
            readonly_memtable_vec_.erase(readonly_memtable_vec_.begin());
          }
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
        readonly_memtable_vec_.push_back(memtable_);
        assert(memtable_.use_count() == 3);
        // 重新创建一个新的 Memtable
        // 释放所有权
        memtable_ = std::make_shared<Memtable>();
        assert(memtable_.use_count() == 1);
        // 设置 memtable 号码
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

void SharedMemtable::PushRemoveReadonlyMemtableContext(const uint32_t n) {
  assert(n >= 0 && n < taskworkers_.size());
  Memtask task = std::make_shared<RemoveReadOnlyMemTableContext>();
  taskworkers_[n]->addTask(task);
}

}  // namespace db

}  // namespace fver