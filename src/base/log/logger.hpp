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

#include "src/base/log/logbuffer.hpp"
#include "src/base/timestamp.hpp"

using ::fver::base::TimeStamp;

namespace fver {
namespace base {

namespace log {

class LogThread;
struct LogImp;

// 'Q' means quit the progress
// TODO: change all the name.
// TODO: need to use fmt::format_to to format the logment
// but now we usually use fver::base::log::Buffer to save the logment
// and to the string => Buffer, we must memcpy once.
// the fmt::format_to need us to
// auto vec_char_out = vector<char>();
// then fmt::format_to(std::back_iterator(vec_char_out), "...");
static const char kLogLevelNums[] = {
    'I', 'T', 'D', 'E', 'W', 'Q',
};

// 10486 = 2097152 / 200;
constexpr static uint64_t kLogThreshold = 10486;

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
    if (!isSync_) {
      timeNow_ = TimeStamp::Now();
      fmt::print(
          "{}{} {}:{}] {}\n", kLogLevelNums[lev], timeNow_.ToFormatTodayNowUs(),
          filename, line,
          fmt::format(fmt::runtime(format_str), std::forward<Args>(args)...));
    }
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
    auto mtx = buf_.getMutex();
    mtx->lock();
    // TODO: should judge the log_vec size
    //  if log.size() >
    //  日志到达水平线直接丢弃, 就不会引发过度扩容
    if ((buf_.curPtr_->capacity() - buf_.curPtr_->size()) < 128) {
      mtx->unlock();
      return;
    }
    fmt::format_to(
        std::back_inserter((*buf_.curPtr_)), "{}{} {}:{}] {}\n",
        kLogLevelNums[lev], timeNow_.ToFormatTodayNowUs(), filename, line,
        fmt::format(fmt::runtime(format_str), std::forward<Args>(args)...));
    mtx->unlock();
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
    // 将发生堆栈信息写入到日志文件中.
    if (lev == kExit) [[unlikely]] {
      return;
    }
  }

  VecLogBuffer* getBufferPtr() { return &buf_; }

  std::vector<char>* SwapBuffer() { return buf_.SwapBuffer(); }

  void setLogLev(int lev) { curLogLevel_ = static_cast<LogLevel>(lev); }

  void setLogToFile() { isSync_ = true; }

  void configInit(std::shared_ptr<std::condition_variable> cond,
                  std::shared_ptr<std::atomic<uint64_t>> sumWrite) {
    cond_ = cond;
    sumWrites_ = sumWrite;
  }

  std::mutex* getMutex() { return buf_.getMutex(); }

  void setBufSize(uint32_t buf_size) {
    auto mtx = buf_.getMutex();
    mtx->lock();
    buf_.head_.reserve(buf_size);
    buf_.tail_.reserve(buf_size);
    mtx->unlock();
    maxBufSize_ = buf_size;
  }

 private:
  // buffer 的水平线
  uint32_t maxBufSize_;
  bool isSync_;
  //
  std::shared_ptr<std::condition_variable> cond_;
  std::shared_ptr<std::atomic<uint64_t>> sumWrites_;
  TimeStamp timeNow_;
  LogLevel curLogLevel_;
  VecLogBuffer buf_;
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