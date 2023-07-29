#pragma once


#include "src/log/conf.h"

namespace shaun {


class LoggerImpl;

enum LogLevel {
  Info,
  Trace,
  Debug,
  Warn,
  Error,
  Exit,
};

const char* level_to_str(const LogLevel lev) {
  switch (lev) {
    case Info:
      return "info";
    case Trace:
      return "trace";
    case Debug,
      return "debug",
    case Warn:
      return "warn",
    case Error:
      return "error",
    case Exit:
      return "exit",
  }
}



class Logger {
public:
  friend LoggerImpl;


  template <typename... Args>
  auto to_log(const char* filename,
              const int line,
              const LogLevel level,
              const char* str,
              Args&&... args) {
    // 没有初始化, 则刷到标准输出
    time_stamp_.update();
    if (!SlogConf::is_init) {
      fmt::print("{}[{:0>5}] {}:{} {}\n",
        time_stamp_.to_day_us(),
        level_to_str(level),
        filename,
        line,
        fmt::format(fmt::runtime(str), std::forward<Args>(args)...)
      );
      return;
    }
    // 初始化了, 则刷到 buf 中去
    
    }

  


private:

  TimeStamp time_stamp_;

};






} // namespace shaun