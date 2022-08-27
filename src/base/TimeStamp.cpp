#include "TimeStamp.hpp"

#include <fmt/format.h>

namespace fver {

namespace base {

std::string TimeStamp::ToFormatToday() {
  time_t seconds =
      static_cast<time_t>(this->sinceepoch_ / kMicroSecondsPerSecond);
  struct tm tm_time;
  gmtime_r(&seconds, &tm_time);
  return fmt::format("{}-{:0>2}-{:0>2}", tm_time.tm_year + 1900,
                     tm_time.tm_mon + 1, tm_time.tm_mday);
}

std::string TimeStamp::ToFormatTodayNowMs() {
  time_t seconds =
      static_cast<time_t>(this->sinceepoch_ / kMicroSecondsPerSecond);
  struct tm tm_time;
  gmtime_r(&seconds, &tm_time);
  auto microseconds = seconds % kMicroSecondsPerSecond;
  return fmt::format("{}-{:0>2}-{:0>2} {}:{:0>2}:{}.{}", tm_time.tm_year + 1900,
                     tm_time.tm_mon + 1, tm_time.tm_mday, tm_time.tm_hour + 8,
                     tm_time.tm_min, tm_time.tm_sec, microseconds);
}

uint64_t TimeStamp::GetTimeofDayMs() {
  timeval now;
  gettimeofday(&now, nullptr);
  return now.tv_sec * 1000L + now.tv_usec / 1000;
}

uint64_t TimeStamp::GetTimeofDayUs() {
  timeval now;
  gettimeofday(&now, nullptr);
  return now.tv_sec * 1000000L + now.tv_usec;
}

TimeStamp TimeStamp::Now() {
  struct timeval tval;
  gettimeofday(&tval, nullptr);
  int64_t seconds = tval.tv_sec;
  return TimeStamp(seconds * kMicroSecondsPerSecond + tval.tv_usec);
}

}  // namespace base
}  // namespace fver