#ifndef SRC_BASE_LOG_LOGGER_H_
#define SRC_BASE_LOG_LOGGER_H_

#include <fmt/core.h>
#include <fmt/format.h>
#include <string.h>

#include <atomic>
#include <cassert>
#include <condition_variable>
#include <iostream>
#include <memory>
#include <string>

#include "src/base/TimeStamp.hpp"
#include "src/base/log/logbuffer.hpp"

using ::fver::base::TimeStamp;

namespace fver {
namespace base {

namespace log {

class LogThread;
struct LogImp;

const std::string kLogLevelNums[] = {
    "I", "T", "D", "E", "W", "E",
};

constexpr static uint64_t kLogThreshold = 2097152 / 200;

class Logger {
 public:
  friend LogImp;
  friend LogThread;
  enum LogLevel {
    kInfo,
    kTrace,
    kDebug,
    kError,
    kWarn,
    kExit,
    KDEBUG_INFO,
    kDEBUG_TRACE,
    kDEBUG_DEBUG,
    kDEBUG_ERROR,
  };
  Logger();
  Logger(uint32_t bufSize);
  ~Logger();
  void ClearSum() { sumWrites_ = 0; }
  template <typename... Args>
  constexpr void LogMent(const char* filename, const int line,
                         const LogLevel lev, const std::string format_str,
                         Args&&... args) {
    if (lev < curLogLevel_) {
      return;
    } else if (lev > kExit) {
// DEBUG 模式, 预编译选择性打印日志
#ifdef DEBUG
  timeNow_ = TimeStamp::Now();
    logstr_ = fmt::format(
        "{}{} {}:{}: {}\n", kLogLevelNums[lev], timeNow_.ToFormatTodayNowMs(),
        filename, line,
        fmt::format(fmt::runtime(format_str), std::forward<Args>(args)...));
    if (!isSync_) {
      fmt::print("{}", logstr_);
    } else {
      if (sumWrites_->load() >= kLogThreshold) {
#ifdef FVER_LOG_DEBUG
        fmt::print("logger notify to the thread");
#endif
        cond_->notify_one();
        sumWrites_ = 0;
      }
#ifdef FVER_LOG_DEBUG
      fmt::print("sumwrites = {}", sumWrites_->load());
#endif
      // 忽略返回值
      buf_.Push(logstr_);
    }
#endif
      return;
    }
    timeNow_ = TimeStamp::Now();
    logstr_ = fmt::format(
        "{}{} {}:{}] {}\n", kLogLevelNums[lev], timeNow_.ToFormatTodayNowUs(),
        filename, line,
        fmt::format(fmt::runtime(format_str), std::forward<Args>(args)...));
    if (!isSync_) {
      fmt::print("{}", logstr_);
    } else {
      if (sumWrites_->load() >= kLogThreshold) {
#ifdef FVER_LOG_DEBUG
        fmt::print("logger notify to the thread");
#endif
        cond_->notify_one();
        sumWrites_ = 0;
      }
#ifdef FVER_LOG_DEBUG
      fmt::print("sumwrites = {}", sumWrites_->load());
#endif
      // 忽略返回值
      buf_.Push(logstr_);
    }
  }

  LogBuffer* getBufferPtr() { return &buf_; }

  Buffer* SwapBuffer() { return buf_.SwapBuffer(); }

  void setLogLev(int lev) { curLogLevel_ = static_cast<LogLevel>(lev); }

  void setLogToFile() {
    fmt::print("{}", logstr_);
    isSync_ = true;
  }

  void ClearOffset(Buffer* buf) { buf_.ClearOffset(buf); }

  void configInit(std::shared_ptr<std::condition_variable> cond,
                  std::shared_ptr<std::atomic<uint64_t>> sumWrite) {
    cond_ = cond;
    sumWrites_ = sumWrite;
  }

  std::mutex* getMutex() { return buf_.getMutex(); }

 private:
  bool isSync_;
  //
  std::shared_ptr<std::condition_variable> cond_;
  std::shared_ptr<std::atomic<uint64_t>> sumWrites_;
  std::string logstr_;
  TimeStamp timeNow_;
  LogLevel curLogLevel_;
  LogBuffer buf_;
  // 当 thread_local 变量被析构时, std::shared_ptr<Logger> 的
  // 引用计数会 -1, 当前的Logger
  bool isHolderByThread_;
};

struct LogImp {
  std::shared_ptr<Logger> log;
  LogImp(LogThread* thread);
  ~LogImp();
};

}  // namespace log
}  // namespace base
}  // namespace fver
#endif