#include "src/base/log/logthread.hpp"

#include <chrono>
#include <condition_variable>
#include <mutex>

#include "src/base/log/logger.hpp"

#define DEBUG

namespace fver {
namespace base {
namespace log {

LogThread::LogThread()
    : loglev_(Logger::kInfo),
      isSync_(false),
      isRunning_(false),
      isAdd_(false),
      sleepPerLoopMs_(100),
      activeLoggern_(0),
      writePerLoop_(0) {}

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
      activeLoggern_ = 0;
      writePerLoop_ = 0;
      isAdd_ = false;
      {
        std::lock_guard<std::mutex> lg(mtx_);
        if (logVectmp_.size() > 0) {
          logworkers_.insert(logworkers_.end(), logVectmp_.begin(),
                             logVectmp_.end());
          logVectmp_.clear();
        }
        isAdd_ = true;
      }
      // 如果有新的Logger加入不切换当前线程.
      // 刷入一次即可
      if (!isAdd_) {
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepPerLoopMs_));
      }
      for (auto& log_iter : logworkers_) {
        auto buf_ptr = log_iter->SwapBuffer();
        writePerLoop_ += buf_ptr->offset_;
        file_.WriteStr(buf_ptr->bufptr_, buf_ptr->offset_);
        buf_ptr->offset_ = 0;
        activeLoggern_++;
      }
      // 根据当前每秒刷入到日志的写入量, 决定当前 loop thread sleep
      // sleepPerLoopMs
      sleepPerLoopMs_ = static_cast<uint64_t>(
          1000.0 - ((static_cast<double>(writePerLoop_) /
                     static_cast<double>(activeLoggern_ * 2048.0)) *
                    1000.0));
      file_.Sync();
#ifdef DEBUG
      fmt::print("activelog = {} writeperloop = {} sleepPerLoopMs = {}\n",
                 activeLoggern_, writePerLoop_, sleepPerLoopMs_);
#endif
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