#ifndef SRC_BASE_LOGGER_H_
#define SRC_BASE_LOGGER_H_

#include <execinfo.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <functional>
#include <memory>
#include <mutex>
#include <ostream>
#include <string>

#include "../NonCopyable.hpp"
#include "../TimeStamp.hpp"
#include "AsyncLoggingThread.hpp"
#include "LogBuffer.hpp"

namespace fver::base::log {

const std::string LogLevelNums[] = {
    "INFO", "TRACE", "DEBUG", "ERROR", "WARN", "EXIT",
};

// default log output function
// usually will put it to the STD_OUT

class Logger : public NonCopyable,
               public std::enable_shared_from_this<Logger> {
 public:
  enum LogLevel : int {
    kInfo,
    kTrace,
    kDebug,
    kError,
    kWarn,
    kExit,
  };
  Logger(AsyncLogThread* logthread);
  Logger(const std::unique_ptr<AsyncLogThread>& thrd);
  Logger();
  ~Logger() = default;
  template <typename... Args>
  void NewLogStateMent(const char* filename, const int line, const LogLevel lev,
                       const std::string& format_str, Args&&... args) {
    if (lev < curloglevel_) [[unlikely]] {
      return;
    }
    time_ = fver::base::TimeStamp::Now();
    // time + loglevel +  filename + line + thread_id + info
    // threadid_ has been cache
    // and the gettimeofday() is vsdo syscall. we can use it everytimes;
    logment_ = fmt::format(
        "{} {} {}:{:0>5} thid: {}: {}\n", time_.ToFormatTodayNowMs(),
        LogLevelNums[lev], filename, line, threadid_,
        fmt::format(fmt::runtime(format_str), std::forward<Args>(args)...));
    if (logway_ == AsyncLogThread::kLogStdOut) {
      fmt::print("{}", logment_);
      if (lev == LogLevel::kExit) {
        exit(-1);
      }
      return;
    } else {
      std::lock_guard<std::mutex> lgk(mtx_);
      curbufptr_->Push(logment_);
    }
    if (IsChangeBuffer()) {
      ChangeBufferPtr();
    }
  }

  void SetCurrentLogLevel(LogLevel lev);
  void SetCurrentLogBufferSize(uint32_t bufsize);
  void SetBufferHorSize(double horsize);
  void SetLogToFile();
  void ChangeBuffer();
  bool IsSync();
  void SyncToFile();
  LogBuffer* GetLogBufferPtr();
  void AddLogTimes();
  void ClearLogTimes();
  bool IsLogToDisk();
  bool IsChangeBuffer();
  void ChangeBufferPtr();

 private:
  // the asynclogginthread will call unlogtimes_ if not ++
  int unlogtimes_;
  pthread_t threadid_;
  AsyncLogThread::LogWay logway_;
  LogLevel curloglevel_;
  std::string logment_;
  std::mutex mtx_;
  double bufhorsize_;
  fver::base::TimeStamp time_;
  // use two buffers provide data condition
  fver::base::log::LogBuffer prebuf_;
  fver::base::log::LogBuffer tailbuf_;
  fver::base::log::LogBuffer* curbufptr_;
};

};  // namespace fver::base::log
// thread_local fver::base::log::Logger logger;

#endif