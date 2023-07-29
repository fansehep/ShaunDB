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



class Logger {
public:
  friend LoggerImpl;


  template <typename... Args>
  void to_log(const char* filename,
              const int line,
              const LogLevel level,
              const char* str,
              Args&&... args) {
    // 没有初始化, 则刷到标准输出
    if (!SlogConf::is_init) {
      
    }
    
    }

  


private:
};






} // namespace shaun