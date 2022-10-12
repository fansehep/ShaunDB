#ifndef SRC_BASE_LOG_LOGGING_H_
#define SRC_BASE_LOG_LOGGING_H_

#include "src/base/log/logger.hpp"
#include "src/base/log/logthread.hpp"
extern fver::base::log::LogThread logthread;
extern thread_local fver::base::log::Logger logger;

enum kLogLevel {
  kInfo,
  kTrace,
  kDebug,
  kError,
  kWarn,
  kExit,
};

namespace fver {
namespace base {
namespace log {

void Init(const std::string logpath, const int lev);

}  // namespace log
}  // namespace base
}  // namespace fver

#define LOG_INFO(str, ...)                                                     \
  logger.LogMent(__FILE__, __LINE__, fver::base::log::Logger::LogLevel::kInfo, \
                 str, ##__VA_ARGS__)
#define LOG_TRACE(str, ...)                                      \
  logger.LogMent(__FILE__, __LINE__,                             \
                 fver::base::log::Logger::LogLevel::kTrace, str, \
                 ##__VA_ARGS__)
#define Log_DEBUG(str, ...)                                      \
  logger.LogMent(__FILE__, __LINE__,                             \
                 fver::base::log::Logger::LogLevel::kDebug, str, \
                 ##__VA_ARGS__)
#define LOG_ERROR(str, ...)                                      \
  logger.LogMent(__FILE__, __LINE__,                             \
                 fver::base::log::Logger::LogLevel::kError, str, \
                 ##__VA_ARGS__)
#define LOG_WARN(str, ...)                                                     \
  logger.LogMent(__FILE__, __LINE__, fver::base::log::Logger::LogLevel::kWarn, \
                 str, ##__VA_ARGS__)
#define LOG_EXIT(str, ...)                                                     \
  logger.LogMent(__FILE__, __LINE__, fver::base::log::Logger::LogLevel::kExit, \
                 str, ##__VA_ARGS__)

#endif