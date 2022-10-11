#ifndef SRC_BASE_LOG_LOGGING_H_
#define SRC_BASE_LOG_LOGGING_H_

#include <iostream>

#include "src/base/log/AsyncLoggingThread.hpp"
#include "src/base/log/Logger.hpp"

extern fver::base::log::AsyncLogThread* glogthread;
extern thread_local fver::base::log::LoggerImp logger;

namespace fver::base::log {

inline void Init(const std::string& logpath, const Logger::LogLevel lev,
                 const uint32_t threadlocalbufsize, const double bufhosize) {
  glogthread = new fver::base::log::AsyncLogThread;
  glogthread->Init(logpath, threadlocalbufsize, lev, bufhosize);
}

}  // namespace fver::base::log
#define LOG_INFO(str, ...)                                                  \
  logger.logptr_->NewLogStateMent(__FILE__, __LINE__,                       \
                                  fver::base::log::Logger::LogLevel::kInfo, \
                                  str, ##__VA_ARGS__)
#define LOG_TRACE(str, ...)                                                  \
  logger.logptr_->NewLogStateMent(__FILE__, __LINE__,                        \
                                  fver::base::log::Logger::LogLevel::kTrace, \
                                  str, ##__VA_ARGS__)
#define Log_DEBUG(str, ...)                                                  \
  logger.logptr_->NewLogStateMent(__FILE__, __LINE__,                        \
                                  fver::base::log::Logger::LogLevel::kDebug, \
                                  str, ##__VA_ARGS__)
#define LOG_ERROR(str, ...)                                                  \
  logger.logptr_->NewLogStateMent(__FILE__, __LINE__,                        \
                                  fver::base::log::Logger::LogLevel::kError, \
                                  str, ##__VA_ARGS__)
#define LOG_WARN(str, ...)                                                  \
  logger.logptr_->NewLogStateMent(__FILE__, __LINE__,                       \
                                  fver::base::log::Logger::LogLevel::kWarn, \
                                  str, ##__VA_ARGS__)
#define LOG_EXIT(str, ...)                                                  \
  logger.logptr_->NewLogStateMent(__FILE__, __LINE__,                       \
                                  fver::base::log::Logger::LogLevel::kExit, \
                                  str, ##__VA_ARGS__)

#endif
