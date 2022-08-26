#ifndef SRC_BASE_LOG_LOGGING_H_
#define SRC_BASE_LOG_LOGGINT_H_

#include <fmt/core.h>
#include <fmt/format.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <functional>
#include <ostream>
#include <string>

#include "../NonCopyable.hpp"
#include "../TimeStamp.hpp"
#include "LogBuffer.hpp"

namespace fver::base::log {

const std::string LogLevelNums[] = {
    "Info", "Trace", "Debug", "Error", "Warn", "Exit",
};

class Logger : public NonCopyable {
 public:
  enum LogLevel : int {
    kInfo,
    kTrace,
    kDebug,
    kError,
    kWarn,
    kExit,
  };
  Logger() = default;
  //~Logger();
  template <typename Locale, typename... T,
            fmt::FMT_ENABLE_IF(fmt::detail::is_locale<Locale>::value)>
  bool NewLogStateMent(const char* filename, int line, LogLevel lev,
                       const Locale& loc, fmt::format_string<T...> fmt,
                       T&&... args) {
    time_ = fver::base::TimeStamp::Now();
    // time + filename + line + loglevel + info
    auto logprefix = fmt::format("{} {} {}", time_.ToFormatTodayNowMs(),
                                 filename, line, LogLevelNums[lev]);
    auto logment = fmt::vformat(loc, fmt::string_view(fmt),
                                fmt::make_format_args(args...));
    printf("%s\n", logprefix + logment);
    return true;
  }
  // bool Clear();
  // bool Expansion(uint32_t bufsize);

 private:
  fver::base::TimeStamp time_;
};
};  // namespace fver::base::log
thread_local fver::base::log::Logger logger;
#define LOG_INFO(str, ...)                                              \
  logger.NewLogStateMent(__FILE__, __LINE__,                            \
                         fver::base::log::Logger::LogLevel::kInfo, str, \
                         ##__VA_ARGS__)
// #define LOG_TRACE(str, args...)                                           \
//   fver::base::log::logger.NewLogStateMent(                                \
//       __FILE__, __LINE__, fver::base::log::Logger::LogLevel::kTrace, str, \
//       __VA_ARGS__)
// #define Log_DEBUG(str, args...)                                           \
//   fver::base::log::logger.NewLogStateMent(                                \
//       __FILE__, __LINE__, fver::base::log::Logger::LogLevel::kDebug, str, \
//       ##__VA_ARGS__)
// #define LOG_ERROR(str, args...)                                           \
//   fver::base::log::logger.NewLogStateMent(                                \
//       __FILE__, __LINE__, fver::base::log::Logger::LogLevel::kError, str, \
//       ##__VA_ARGS__)
// #define LOG_WARN(str, args...)                                           \
//   fver::base::log::logger.NewLogStateMent(                               \
//       __FILE__, __LINE__, fver::base::log::Logger::LogLevel::kWarn, str, \
//       ##__VA_ARGS__)
// #define LOG_EXIT(str, args...)                                           \
//   fver::base::log::logger.NewLogStateMent(                               \
//       __FILE__, __LINE__, fver::base::log::Logger::LogLevel::kExit, str, \
//       ##__VA_ARGS__)

#endif