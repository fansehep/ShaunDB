#ifndef _UBERS_LOGGING_H_
#define _UBERS_LOGGING_H_

#include <string.h>
#include "LogStream.h"

namespace UBERS
{

//* 日志生成
class Logger
{
public:
  enum LogLevel
  {
    TRACE, //* 较低日志等级
    DEBUG, //* 细粒度信息事件，主要用来调试应用程序
    INFO,  //* 程序运行时的重要信息
    WARN,  //* 会出现潜在错误的情形，
    ERROR, //* 虽然发生错误事件，但不影响事件运行
    FATAL, //* 严重事件，直接将应用程序退出
    OFF,   //* 最高等级
  };

  class SourceFile
  {
  public:
    template<int N>
    inline SourceFile(const char (&arr)[N]) : data_(arr), size_(N - 1)
    {
      const char* slash = strrchr(data_, '/');
      if(slash)
      {
        data_ = slash + 1;
        size_  -= (data_ - arr);
      }
    }

    explicit SourceFile(const char* filename)
        : data_(filename)
    {
      const char* slash = strrchr(filename, '/');
      if(slash)
      {
        data_ = slash + 1;
      }
      size_ = strlen(data_);
    }
    const char* data_;
    size_t size_;
    };

    Logger(SourceFile file, int line);
    Logger(SourceFile file, int line, LogLevel level);
    Logger(SourceFile file, int line, LogLevel level, const char* func);
    Logger(SourceFile file, int line, bool toAbort);
    ~Logger();

    LogStream& stream() { return impl_.stream_;}

    static LogLevel logLevel();
    static void setLogLevel(LogLevel level);

    typedef void (*OutputFunc)(const char* msg, size_t len);
    typedef void (*FlushFunc)();

    static void setOutPut(OutputFunc);
    static void setFlush(FlushFunc);
  private:
    class Impl
    {
    public:
      using LogLevel = Logger::LogLevel;
      Impl(LogLevel level, int old_errno, const SourceFile& file, int line);
      void finish();

      LogStream stream_;
      LogLevel level_;
      int line_;
      SourceFile Basename_;
    };
    Impl impl_;
};

extern Logger::LogLevel g_logLevel;

inline Logger::LogLevel Logger::logLevel()
{
  return g_logLevel;
}

#define LOG_DEBUG  \
  if(UBERS::Logger::logLevel() <= UBERS::Logger::DEBUG) \
    UBERS::Logger(__FILE__, __LINE__, UBERS::Logger::DEBUG, __func__).stream()
#define LOG_INFO   \
  if(UBERS::Logger::logLevel() <= UBERS::Logger::INFO) \
    UBERS::Logger(__FILE__, __LINE__, UBERS::Logger::INFO) \
#define LOG_WARN UBERS::Logger(__FILE__, __LINE__, UBERS::Logger::WARN).stream()
#define LOG_ERROR UBERS::Logger(__FILE__, __LINE__, UBERS::Logger::ERROR).stream()
#define LOG_FATAL UBERS::Logger(__FILE__, __LINE__, UBERS::Logger::FATAL).stream()
#define LOG_OFF UBERS::Logger(__FILE__, __LINE__, UBERS::Logger::OFF).stream()        

const char* strerror_tl(int savedErrno);
}

#endif