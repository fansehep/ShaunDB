#ifndef SRC_BASE_LOG_LOGGING_H_
#define SRC_BASE_LOG_LOGGING_H_

#include <iostream>

#include "AsyncLoggingThread.hpp"
#include "Logger.hpp"
namespace fver::base::log {

AsyncLogThread glogthread;
thread_local Logger logger(&glogthread);

inline void Init(const std::string& logpath, const Logger::LogLevel lev,
                 const uint32_t threadlocalbufsize, const double bufhosize) {
  glogthread.Init(logpath, threadlocalbufsize, lev, bufhosize);
}

}  // namespace fver::base::log
#define LOG_INFO(str, ...)                                               \
  fver::base::log::logger.NewLogStateMent(                               \
      __FILE__, __LINE__, fver::base::log::Logger::LogLevel::kInfo, str, \
      ##__VA_ARGS__)
#define LOG_TRACE(str, ...)                                               \
  fver::base::log::logger.NewLogStateMent(                                \
      __FILE__, __LINE__, fver::base::log::Logger::LogLevel::kTrace, str, \
      ##__VA_ARGS__)
#define Log_DEBUG(str, ...)                                               \
  fver::base::log::logger.NewLogStateMent(                                \
      __FILE__, __LINE__, fver::base::log::Logger::LogLevel::kDebug, str, \
      ##__VA_ARGS__)
#define LOG_ERROR(str, ...)                                               \
  fver::base::log::logger.NewLogStateMent(                                \
      __FILE__, __LINE__, fver::base::log::Logger::LogLevel::kError, str, \
      ##__VA_ARGS__)
#define LOG_WARN(str, ...)                                               \
  fver::base::log::logger.NewLogStateMent(                               \
      __FILE__, __LINE__, fver::base::log::Logger::LogLevel::kWarn, str, \
      ##__VA_ARGS__)
#define LOG_EXIT(str, ...)                                               \
  fver::base::log::logger.NewLogStateMent(                               \
      __FILE__, __LINE__, fver::base::log::Logger::LogLevel::kExit, str, \
      ##__VA_ARGS__)

#endif
