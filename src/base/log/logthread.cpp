#include "src/base/log/logthread.hpp"

#include <chrono>
#include <condition_variable>
#include <mutex>

#include "src/base/log/logger.hpp"

namespace fver {
namespace base {
namespace log {

LogThread::LogThread()
    : loglev_(Logger::kInfo), isSync_(false), isRunning_(false) {}

void LogThread::AddLogger(Logger* log) {
  std::lock_guard<std::mutex> lg(mtx_);
  log->setLogLev(loglev_);
  if (true == isRunning_) {
    log->setLogToFile();
  }
  logVectmp_.push_back(log);
}

void LogThread::Init(const std::string& logpath, const int lev) {
  file_.SetLogPath(logpath);
  loglev_ = lev;
  isSync_ = true;
  isRunning_ = true;
  for (auto& iter : logVectmp_) {
    iter->setLogToFile();
  }
  syncThread_ = std::thread([&]() {
    while (isRunning_) {
      //* 将新加入的Logger加入到 logVectmp 中
      {
        std::lock_guard<std::mutex> lg(mtx_);
        if (logVectmp_.size() > 0) {
          logworkers_.insert(logworkers_.end(), logVectmp_.begin(),
                             logVectmp_.end());
          logVectmp_.clear();
        }
      }

      for (auto& log_iter : logworkers_) {
        auto buf_ptr = log_iter->SwapBuffer();
        file_.WriteStr(buf_ptr->bufptr_, buf_ptr->offset_);
        buf_ptr->offset_ = 0;
      }
    }

    // need stop to log
    if (!isRunning_) {
      for (auto& log_iter : logworkers_) {
        auto buf_ptr = log_iter->SwapBuffer();
        file_.WriteStr(buf_ptr->bufptr_, buf_ptr->offset_);
        buf_ptr->offset_ = 0;
      }

      for (auto& log_iter : logworkers_) {
        auto buf_ptr = log_iter->SwapBuffer();
        file_.WriteStr(buf_ptr->bufptr_, buf_ptr->offset_);
        buf_ptr->offset_ = 0;
      }
    }
    file_.Sync();
  });
}

void LogThread::Stop() {
  isRunning_ = false;
  syncThread_.join();
}

LogThread::~LogThread() { Stop(); }

}  // namespace log

}  // namespace base

}  // namespace fver