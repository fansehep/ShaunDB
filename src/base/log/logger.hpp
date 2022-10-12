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

const std::string kLogLevelNums[] = {
    "INFO", "TRACE", "DEBUG", "ERROR", "WARN", "EXIT",
};

class Logger {
 public:
  enum LogLevel {
    kInfo,
    kTrace,
    kDebug,
    kError,
    kWarn,
    kExit,
  };
  Logger();
  Logger(LogThread* logthread, uint32_t bufsize = 2048);
  Logger(uint32_t bufSize);
  ~Logger() = default;
  void ClearSum() { sumWrites_ = 0; }
  template <typename... Args>
  void LogMent(const char* filename, const int line, const LogLevel lev,
               const std::string format_str, Args&&... args) {
    if (lev < curLogLevel_) {
      return;
    }
    timeNow_ = TimeStamp::Now();
    logstr_ = fmt::format(
        "{} {} {}:{}: {}\n", timeNow_.ToFormatTodayNowMs(), kLogLevelNums[lev],
        filename, line,
        fmt::format(fmt::runtime(format_str), std::forward<Args>(args)...));
    if (!isSync_) {
      fmt::print("{}", logstr_);
    } else {
      fmt::print("sumwrites = {}", sumWrites_);
      sumWrites_ += logstr_.size();
      buf_.Push(logstr_);
    }
  }

  LogBuffer* getBufferPtr() { return &buf_; }

  Buffer* SwapBuffer() {
    sumWrites_ = 0;
    return buf_.SwapBuffer();
  }

  void setLogLev(int lev) { curLogLevel_ = static_cast<LogLevel>(lev); }

  void setLogToFile() {
    fmt::print("{}", logstr_);
    isSync_ = true;
  }

  uint32_t getSum() { return sumWrites_; }

  void ClearOffset(Buffer* buf) { buf_.ClearOffset(buf); }

 private:
  bool isSync_;
  std::atomic<uint32_t> sumWrites_;
  std::string logstr_;
  TimeStamp timeNow_;
  LogLevel curLogLevel_;
  LogBuffer buf_;
};

}  // namespace log
}  // namespace base
}  // namespace fver
#endif