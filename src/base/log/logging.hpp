#ifndef SRC_BASE_LOG_LOGGING_H_
#define SRC_BASE_LOG_LOGGING_H_

#include "src/base/log/logger.hpp"
#include "src/base/log/logthread.hpp"
extern fver::base::log::LogThread logthread;
extern thread_local fver::base::log::LogImp logimp;

enum kLogLevel {
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

static constexpr uint32_t kDefaultMaxLogBufSize = 16 * 1024 * 1024;

namespace fver {
namespace base {
namespace log {

void Init(const std::string& logpath, const int lev, const bool isSync,
          const std::string& logPrename,
          const uint32_t log_bug_size = kDefaultMaxLogBufSize);

}  // namespace log
}  // namespace base
}  // namespace fver

#define LOG_INFO(str, ...)                                           \
  logimp.log->LogMent(__FILE_NAME__, __LINE__,                       \
                      fver::base::log::Logger::LogLevel::kInfo, str, \
                      ##__VA_ARGS__)
#define LOG_TRACE(str, ...)                                           \
  logimp.log->LogMent(__FILE_NAME__, __LINE__,                        \
                      fver::base::log::Logger::LogLevel::kTrace, str, \
                      ##__VA_ARGS__)
#define LOG_DEBUG(str, ...)                                           \
  logimp.log->LogMent(__FILE_NAME__, __LINE__,                        \
                      fver::base::log::Logger::LogLevel::kDebug, str, \
                      ##__VA_ARGS__)
#define LOG_ERROR(str, ...)                                           \
  logimp.log->LogMent(__FILE_NAME__, __LINE__,                        \
                      fver::base::log::Logger::LogLevel::kError, str, \
                      ##__VA_ARGS__)
#define LOG_WARN(str, ...)                                           \
  logimp.log->LogMent(__FILE_NAME__, __LINE__,                       \
                      fver::base::log::Logger::LogLevel::kWarn, str, \
                      ##__VA_ARGS__)
#define LOG_EXIT(str, ...)                                           \
  logimp.log->LogMent(__FILE_NAME__, __LINE__,                       \
                      fver::base::log::Logger::LogLevel::kExit, str, \
                      ##__VA_ARGS__)

// DLOG_* 只在 编译启用 -DDEBUG 才会有输出

#define DLOG_INFO(str, ...)                                                \
  logimp.log->LogMent(__FILE_NAME__, __LINE__,                             \
                      fver::base::log::Logger::LogLevel::KDEBUG_INFO, str, \
                      ##__VA_ARGS__)
#define DLOG_TRACE(str, ...)                                                \
  logimp.log->LogMent(__FILE_NAME__, __LINE__,                              \
                      fver::base::log::Logger::LogLevel::kDEBUG_TRACE, str, \
                      ##__VA_ARGS__)
#define DLog_DEBUG(str, ...)                                                \
  logimp.log->LogMent(__FILE_NAME__, __LINE__,                              \
                      fver::base::log::Logger::LogLevel::kDEBUG_DEBUG, str, \
                      ##__VA_ARGS__)
#define DLOG_ERROR(str, ...)                                                \
  logimp.log->LogMent(__FILE_NAME__, __LINE__,                              \
                      fver::base::log::Logger::LogLevel::kDEBUG_ERROR, str, \
                      ##__VA_ARGS__)

#endif