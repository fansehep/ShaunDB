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

#include "../TimeStamp.hpp"
#include "AsyncLoggingThread.hpp"
#include "LogBuffer.hpp"
#include "ThreadLocalBuffer.hpp"
namespace fver::base::log {

const std::string LogLevelNums[] = {
    "INFO", "TRACE", "DEBUG", "ERROR", "WARN", "EXIT",
};

// default log output function
// usually will put it to the STD_OUT

struct LoggerImp;

class Logger {
 public:
  friend struct LoggerImp;
  enum LogLevel : int {
    kInfo,
    kTrace,
    kDebug,
    kError,
    kWarn,
    kExit,
  };
  Logger();
  Logger(uint32_t bufsize);
  ~Logger() = default;
  template <typename... Args>
  void NewLogStateMent(const char* filename, const int line, const LogLevel lev,
                       const std::string& format_str, Args&&... args) {
    if (lev < curloglevel_) [[unlikely]] {
      return;
    }
#ifdef DEBUG
    fmt::print("{} {} curlogway_ : {} curloglevel_: {} \n", __FILE__, __LINE__,
               logway_, curloglevel_);
#endif
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
#ifdef DEBUG
      fmt::print("{} {} logment: {}\n", __FILE__, __LINE__, logment_);
#endif
      buf_->Push(logment_);
    }
  }

  void SetCurrentLogLevel(LogLevel lev);
  void AddLogTimes();
  void ClearLogTimes();
  void ChangeBufferPtr();
  bool IsChangeBuffer();
  int GetUnlogTimes();
  void SetLogToFile();
  void SetLogToStdOut();
  // timeout must log to disk
  bool IsTimeOut();
  // >= default 80% must log to disk
  bool IsFillThresold();
  char* GetBufPtr() {return buf_->GetBeginPtr();}
  int GetBufSize() {return buf_->GetSize();}
  void ClearTmpBuf() {buf_->ClearTmpBuf();}
 private:
  pthread_t threadid_;
  AsyncLogThread::LogWay logway_;
  LogLevel curloglevel_;
  std::string logment_;
  fver::base::TimeStamp time_;
  std::shared_ptr<ThreadLocalBuffer> buf_;
};

struct LoggerImp {
  Logger* logptr_;
  LoggerImp() { logptr_ = new Logger(); }
  LoggerImp(AsyncLogThread* logthread) {
    logptr_ = new Logger(LogBuffer::kMidBufferSize);
    if (logthread) {
#ifdef DEBUG
      fmt::print("{} {} logger ready to push\n", __FILE__, __LINE__);
#endif
      logthread->PushLogWorker(this);
    } else {
#ifdef DEBUG
      fmt::print("{} {} logger set asynclogthread::logstdout\n", __FILE__,
                 __LINE__);
#endif
      logptr_->SetLogToStdOut();
    }
  }
};

};  // namespace fver::base::log
// thread_local fver::base::log::Logger logger;

#endif