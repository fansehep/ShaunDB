#include "Logging.h"
#include <stdio.h>
#include <string.h>
#include "CurrentThread.h"
#include "TimeStamp.h"
namespace UBERS
{
  thread_local char t_errnobuf[512];

const char* strerror_tl(int SavedErrno)
{
  return strerror_r(SavedErrno, t_errnobuf, sizeof(t_errnobuf));
}

Logger::LogLevel initLogLevel()
{
  return Logger::INFO;
}
Logger::LogLevel g_logLevel = initLogLevel();

const char* LogLevelName[7] = { \
  "TRACE ",
  "DEBUG ",
  "INFO ", 
  "WARN ",
  "ERROR ",
  "FATAL ",
  "OFF ",
};

class T
{
public:
  T(const char* str, unsigned len) : str_(str), len_(len)
  {
    assert(strlen(str) == len_);
  }
  const char* str_;
  const unsigned len_;
};

LogStream& operator << (LogStream& s, T v)
{
  s.append(v.str_, v.len_);
  return s;
}

LogStream& operator << (LogStream& s, const Logger::SourceFile& v)
{
  s.append(v.data_, v.size_);
  return s;
}

//* 默认输出函数，写入 STDOUT
void defaultOutput(const char* msg, size_t len)
{
  size_t n = fwrite(msg, 1, len, stdout);
}
//* 默认刷新函数
void defaultFlush()
{
  fflush(stdout);
}

Logger::OutputFunc g_output = defaultOutput;
Logger::FlushFunc g_flush = defaultFlush;
}// namespace UBERS

using namespace UBERS;

Logger::Impl::Impl(LogLevel level, int SavedErrno, const SourceFile& file, int line)
  : stream_(), level_(level), line_(line), Basename_(file)
{
  CurrentThread::tid();
  stream_ << TimeStamp::now().ToFormattedString() << CurrentThread::tidString() << " ";
  stream_ << LogLevelName[level];
  if(SavedErrno != 0)
  {
    stream_ << TimeStamp::now().ToFormattedString() << strerror_tl(SavedErrno) << " (errno = " << SavedErrno << " ) ";
  }
}

void Logger::Impl::finish()
{
  stream_ << " - " << Basename_.data_ << " : " << line_ << '\n';
}

Logger::Logger(SourceFile file, int line) : impl_(INFO, 0, file, line)
{
  
}
Logger::Logger(SourceFile file, int line, LogLevel level, const char* func)
      : impl_(level, 0, file, line)
{
  impl_.stream_ << func << ' ';
}

Logger::Logger(SourceFile file, int line, LogLevel level)
  : impl_(level, 0, file, line)
{

}


Logger::Logger(SourceFile file, int line, bool toAbort)
  : impl_(toAbort ? FATAL : ERROR, errno, file, line)
{
}

Logger::~Logger()
{
  impl_.finish();
  //* 获得 Impl 的内部 Buffer
  const LogStream::Buffer& buf(stream().buffer());
  //*调用默认输出函数
  g_output(buf.data(), buf.length());
  if(impl_.level_ == FATAL)
  {
    g_flush();
    abort();
  }
}

void Logger::setLogLevel(Logger::LogLevel level)
{
  g_logLevel = level;
}

void Logger::setOutPut(OutputFunc out)
{
  g_output = out;
}

void Logger::setFlush(FlushFunc flush)
{
  g_flush = flush;
}















