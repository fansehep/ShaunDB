#ifndef SRC_BASE_LOG_LOGGING_H_
#define SRC_BASE_LOG_LOGGING_H_

#include <iostream>

#include "AsyncLoggingThread.hpp"
#include "Logger.hpp"

std::unique_ptr<fver::base::log::AsyncLogThread> glogthread = nullptr;
thread_local std::shared_ptr<fver::base::log::Logger> logger = std::make_shared<fver::base::log::Logger>(glogthread);
namespace fver::base::log {

inline void Init(const char* logpath, const Logger::LogLevel lev,
                 const uint32_t threadlocalbufsize, const double bufhosize) {
  glogthread = std::make_unique<fver::base::log::AsyncLogThread>();
  glogthread->Init(logpath, threadlocalbufsize, lev, bufhosize);
}

}  // namespace fver::base::log
#define LOG_INFO(str, ...)                                              \
  logger->NewLogStateMent(__FILE__, __LINE__,                            \
                         fver::base::log::Logger::LogLevel::kInfo, str, \
                         ##__VA_ARGS__)
#define LOG_TRACE(str, ...)                                              \
  logger->NewLogStateMent(__FILE__, __LINE__,                             \
                         fver::base::log::Logger::LogLevel::kTrace, str, \
                         ##__VA_ARGS__)
#define Log_DEBUG(str, ...)                                              \
  logger->NewLogStateMent(__FILE__, __LINE__,                             \
                         fver::base::log::Logger::LogLevel::kDebug, str, \
                         ##__VA_ARGS__)
#define LOG_ERROR(str, ...)                                              \
  logger->NewLogStateMent(__FILE__, __LINE__,                             \
                         fver::base::log::Logger::LogLevel::kError, str, \
                         ##__VA_ARGS__)
#define LOG_WARN(str, ...)                                              \
  logger->NewLogStateMent(__FILE__, __LINE__,                            \
                         fver::base::log::Logger::LogLevel::kWarn, str, \
                         ##__VA_ARGS__)
#define LOG_EXIT(str, ...)                                              \
  logger->NewLogStateMent(__FILE__, __LINE__,                            \
                         fver::base::log::Logger::LogLevel::kExit, str, \
                         ##__VA_ARGS__)

#endif
