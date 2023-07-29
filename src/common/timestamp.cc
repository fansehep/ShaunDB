#include "src/common/timestamp.h"
#include "fmt/format.h"
extern "C" {
#include <time.h>
#include <sys/time.h>
}

namespace shaun {
constexpr static int kMillSec = 1000000;

std::string TimeStamp::to_day() {
  sinceepoch_ = tval_.tv_sec  * kMicroSecondsPerSecond + tval_.tv_usec;;
  time_t seconds =
      static_cast<time_t>(this->sinceepoch_ / kMicroSecondsPerSecond);
  struct tm tm_time;
  gmtime_r(&seconds, &tm_time);
  return fmt::format("{}-{:0>2}-{:0>2}", tm_time.tm_year + 1900,
                     tm_time.tm_mon + 1, tm_time.tm_mday);
}

std::string TimeStamp::to_day_ms() {
  sinceepoch_ = tval_.tv_sec  * kMicroSecondsPerSecond + tval_.tv_usec;;
  time_t seconds =
      static_cast<time_t>(this->sinceepoch_ / kMicroSecondsPerSecond);
  struct tm tm_time;
  gmtime_r(&seconds, &tm_time);
  auto microseconds = seconds % kMicroSecondsPerSecond;
  return fmt::format("{}-{:0>2}-{:0>2} {}:{:0>2}:{}.{}", tm_time.tm_year + 1900,
                     tm_time.tm_mon + 1, tm_time.tm_mday, tm_time.tm_hour + 8,
                     tm_time.tm_min, tm_time.tm_sec, microseconds);
}

std::string TimeStamp::to_day_us() {
  sinceepoch_ = tval_.tv_sec  * kMicroSecondsPerSecond + tval_.tv_usec;
  auto seconds =
      static_cast<time_t>(this->sinceepoch_ / kMicroSecondsPerSecond);
  struct tm tm_time;
  gmtime_r(&seconds, &tm_time);
  auto microseconds = seconds % kMicroSecondsPerSecond;
  return fmt::format("{}-{:0>2}-{:0>2} {:0>2}:{:0>2}:{:0>2}.{} {:0>5}",
                     tm_time.tm_year + 1900, tm_time.tm_mon + 1,
                     tm_time.tm_mday, tm_time.tm_hour + 8, tm_time.tm_min,
                     tm_time.tm_sec, microseconds,
                     (tval_.tv_sec  * kMillSec + tval_.tv_usec % 100000));
}

std::string TimeStamp::to_day_min() {
  sinceepoch_ = tval_.tv_sec * kMicroSecondsPerSecond + tval_.tv_usec ;
  time_t seconds =
      static_cast<time_t>(this->sinceepoch_ / kMicroSecondsPerSecond);
  struct tm tm_time;
  gmtime_r(&seconds, &tm_time);
  return fmt::format("{}-{:0>2}-{:0>2}-{}:{:0>2}-{:0>2}", tm_time.tm_year + 1900,
                     tm_time.tm_mon + 1, tm_time.tm_mday, tm_time.tm_hour + 8,
                     tm_time.tm_min, rand());
}

uint64_t TimeStamp::time_of_day_ms() {
  return tval_.tv_sec * 1000L + tval_.tv_usec / 1000;
}

uint64_t TimeStamp::time_of_day_us() {
  return tval_.tv_sec * 1000000L + tval_.tv_usec;
}

void TimeStamp::update() {
  gettimeofday(&tval_, nullptr);
}

}