#include "src/base/log/logthread.hpp"

#include <asm-generic/errno-base.h>
#include <pthread.h>
#include <sys/signal.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>

#include "src/base/backtrace.hpp"
#include "src/base/log/logger.hpp"
#include "src/base/log/logging.hpp"

namespace fver {
namespace base {
namespace log {

void SignalExecute(int signal) {
  switch (signal) {
    case SIGINT: {
      auto error_log = fver::base::stackTrace(true);
      LOG_ERROR("SIGINT: {}", error_log);
      exit(-1);
    }
    case SIGQUIT: {
      auto error_log = fver::base::stackTrace(true);
      LOG_ERROR("SIGQUIT:{}", error_log);
      exit(-1);
    }
  }
}

LogThread::LogThread()
    : loglev_(Logger::kInfo), isSync_(false), isRunning_(false) {
  cond_ = std::make_shared<std::condition_variable>();
  sumWrites_ = std::make_shared<std::atomic<uint64_t>>(0);

  // 信号注册
  signal(SIGINT, SignalExecute);
  signal(SIGQUIT, SignalExecute);
}

void LogThread::AddLogger(std::shared_ptr<Logger> log) {
  std::lock_guard<std::mutex> lg(mtx_);
  log->setLogLev(loglev_);
  if (true == isRunning_) {
    log->setLogToFile();
  }
  log->configInit(cond_, sumWrites_);
  logVectmp_.push_back(log);
  // 不要阻塞, 快速将新加入的 log 加入到 logVectmp_ 中管理.
  if (true == isRunning_) {
    cond_->notify_one();
  }
}

void LogThread::Init(const std::string& logpath, const int lev,
                     const bool isSync, const std::string& logPrename) {
  file_.SetLogPath(logpath, logPrename);
  loglev_ = lev;
  isSync_ = isSync;
  isRunning_ = true;
  // 如果不启用sync
  if (false == isSync) {
    syncThread_ = std::thread([&]() {
      while (isRunning_) {
        // 将新加入的Logger加入到 logVectmp 中
        {
          std::lock_guard<std::mutex> lg(mtx_);
          if (logVectmp_.size() > 0) {
            logworkers_.insert(logworkers_.end(), logVectmp_.begin(),
                               logVectmp_.end());
            logVectmp_.clear();
          }
        }

        std::unique_lock<std::mutex> lock(logMutex_);
        cond_->wait_for(lock, std::chrono::seconds(1));
        const int N = static_cast<int>(logworkers_.size());

        for (int i = 0; i < N; i++) {
          auto log_iter = logworkers_[i];
          auto mlock = log_iter->getMutex();
          mlock->lock();
          auto buf_ptr = log_iter->SwapBuffer();
          mlock->unlock();
          if (buf_ptr->empty()) {
            // thread_local 变量被析构, 我们需要将被释放的 Logger 移除
            // 但释放前必须, 将他的两个buf 刷入到磁盘中
            if (false == log_iter->isHolderByThread_) {
              mlock->lock();
              auto tbuf_ptr = log_iter->SwapBuffer();
              mlock->unlock();

              if (0 == tbuf_ptr->size()) {
                // 偏移量也为 0, 所以直接删除即可
                // 让他与最后一个交换, 再删除
                std::swap(logworkers_[i], logworkers_.back());
                logworkers_.pop_back();
              } else {
                file_.WriteStr(&((*tbuf_ptr)[0]), tbuf_ptr->size());
                std::swap(logworkers_[i], logworkers_.back());
                logworkers_.pop_back();
              }
            }
            continue;
          }
          file_.WriteStr(&((*buf_ptr)[0]), buf_ptr->size());
          buf_ptr->clear();
        }
      }

      // 刷新所有的 buf
      if (!isRunning_) {
        for (auto& log_iter : logworkers_) {
          auto mlock = log_iter->getMutex();
          mlock->lock();
          auto buf_ptr = log_iter->SwapBuffer();
          if (buf_ptr->empty()) {
            mlock->unlock();
            continue;
          }
          file_.WriteStr(&((*buf_ptr)[0]), buf_ptr->size());
          buf_ptr->clear();
          mlock->unlock();
        }

        for (auto& log_iter : logworkers_) {
          auto mlock = log_iter->getMutex();
          mlock->lock();
          auto buf_ptr = log_iter->SwapBuffer();
          if (buf_ptr->empty()) {
            mlock->unlock();
            continue;
          }
          file_.WriteStr(&((*buf_ptr)[0]), buf_ptr->size());
          buf_ptr->clear();
          mlock->unlock();
        }
      }
    });
  } else {
    syncThread_ = std::thread([&]() {
      while (isRunning_) {
        // 将新加入的Logger加入到 logVectmp 中
        {
          std::lock_guard<std::mutex> lg(mtx_);
          if (logVectmp_.size() > 0) {
            logworkers_.insert(logworkers_.end(), logVectmp_.begin(),
                               logVectmp_.end());
            logVectmp_.clear();
          }
        }

        std::unique_lock<std::mutex> lock(logMutex_);
        cond_->wait_for(lock, std::chrono::milliseconds(750));
        const int N = static_cast<int>(logworkers_.size());

        for (int i = 0; i < N; i++) {
          auto log_iter = logworkers_[i];
          auto mlock = log_iter->getMutex();
          mlock->lock();
          auto buf_ptr = log_iter->SwapBuffer();
          mlock->unlock();
          if (buf_ptr->empty()) {
            // thread_local 变量被析构, 我们需要将被释放的 Logger 移除
            // 但释放前必须, 将他的两个buf 刷入到磁盘中
            if (false == log_iter->isHolderByThread_) {
              mlock->lock();
              auto tbuf_ptr = log_iter->SwapBuffer();
              mlock->unlock();

              if (tbuf_ptr->empty()) {
                // 偏移量也为 0, 所以直接删除即可
                // 让他与最后一个交换, 再删除
                std::swap(logworkers_[i], logworkers_.back());
                logworkers_.pop_back();
              } else {
                file_.WriteStr(&((*buf_ptr)[0]), buf_ptr->size());
                std::swap(logworkers_[i], logworkers_.back());
                logworkers_.pop_back();
              }
            }
            continue;
          }
          file_.WriteStr(&((*buf_ptr)[0]), buf_ptr->size());
          buf_ptr->clear();
        }
        file_.Sync();
      }

      // 刷新所有的 buf
      if (!isRunning_) {
        for (auto& log_iter : logworkers_) {
          auto mlock = log_iter->getMutex();
          mlock->lock();
          auto buf_ptr = log_iter->SwapBuffer();
          if (buf_ptr->empty()) {
            mlock->unlock();
            continue;
          }
          file_.WriteStr(&((*buf_ptr)[0]), buf_ptr->size());
          buf_ptr->clear();
          mlock->unlock();
        }

        for (auto& log_iter : logworkers_) {
          auto mlock = log_iter->getMutex();
          mlock->lock();
          auto buf_ptr = log_iter->SwapBuffer();
          if (buf_ptr->empty()) {
            mlock->unlock();
            continue;
          }
          file_.WriteStr(&((*buf_ptr)[0]), buf_ptr->size());
          buf_ptr->clear();
          mlock->unlock();
        }
      }
    });
  }
}

void LogThread::Stop() {
  isRunning_ = false;
  cond_->notify_one();
  // join 一个没有开启的线程是很危险的
  if (syncThread_.joinable()) {
    syncThread_.join();
  }
}

LogThread::~LogThread() { Stop(); }

}  // namespace log

}  // namespace base

}  // namespace fver