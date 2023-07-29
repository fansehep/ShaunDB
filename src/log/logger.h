#pragma once

#include "fmt/core.h"
#include "fmt/format.h"
#include "src/common/timestamp.h"
#include "src/log/conf.h"
#include <assert.h>

namespace shaun {

class LoggerImpl;

enum LogLevel {
  Info,
  Trace,
  Debug,
  Warn,
  Error,
  Exit,
#ifdef DEBUG
  DInfo,
  DWarn,
  Dtrace,
#endif
};

const char *level_to_str(const LogLevel lev) {
  switch (lev) {
  case Info:
    return "info";
  case Trace:
    return "trace";
  case Debug:
    return "debug";
  case Warn:
    return "warn";
  case Error:
    return "error";
  case Exit:
    return "exit";
#ifdef DEBUG
  case DInfo:
    return "dinfo";
  case DWarn:
    return "dwarn";
  case Dtrace:
    return "dtrace";
#endif
  default:
    return "unknown";
  }
}

class Logger {
public:
  friend LoggerImpl;

  template <typename... Args>
  auto to_log(const char *filename, const int line, const LogLevel level,
              const char *str, Args &&...args) {
    if (level < SlogConf::CurrentLogLevel) {
      return;
    }
    // 没有初始化, 则刷到标准输出
    time_stamp_.update();
    if (!SlogConf::is_init) {
      fmt::print("{} [{}] {}:{} {}\n", time_stamp_.to_day_ms(),
                 level_to_str(level), filename, line,
                 fmt::format(fmt::runtime(str), std::forward<Args>(args)...));
      return;
    }
    // 初始化了, 则刷到 buf 中去
  }

private:
  TimeStamp time_stamp_;
};

class LoggerImpl {
public:
  LoggerImpl() {
    logger_ = std::make_shared<Logger>();
    assert(logger_ != nullptr);
  }
  auto &get_logger() { return logger_; }

private:
  std::shared_ptr<Logger> logger_;
};

} // namespace shaun
