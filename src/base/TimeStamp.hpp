#ifndef SRC_BASE_TIMESTAMP_H_
#define SRC_BASE_TIMESTAMP_H_

#include <sys/time.h>
#include <time.h>

#include <chrono>
#include <compare>
#include <string>

namespace fver {
namespace base {
class TimeStamp {
 public:
  constexpr TimeStamp() : sinceepoch_(0) {}
  ~TimeStamp() = default;
  TimeStamp(std::time_t newtime) : sinceepoch_(newtime) {}
  TimeStamp(const TimeStamp& lef) : sinceepoch_(lef.sinceepoch_) {}
  // return 2022-08-26
  std::string ToFormatToday();
  // return 2022-08-26 17:10 45.505045
  std::string ToFormatTodayNowMs();
  static TimeStamp Now();

 private:
  auto GetTimeofDayUs() -> uint64_t;
  auto GetTimeofDayMs() -> uint64_t;
  static constexpr uint64_t kMilliSecondsPerSecond = 1000ull;
  static constexpr uint64_t kMicroSecondsPerSecond = 1000ull * 1000;

  std::time_t sinceepoch_;
};

}  // namespace base
}  // namespace fver

#endif