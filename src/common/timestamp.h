#pragma once

namespace shaun {

extern "C" {
#include <sys/time.h>
#include <time.h>
}

#include <string>

class TimeStamp {
public:
  constexpr TimeStamp() = default;
  ~TimeStamp() = default;
  TimeStamp(const uint64_t time_sec, const uint64_t time_usec)
      : time_sec_(time_sec), time_usec_(time_usec) {}
  TimeStamp(std::time_t newtime) : sinceepoch_(newtime) {}
  TimeStamp(const TimeStamp &lef) : sinceepoch_(lef.sinceepoch_) {}
  TimeStamp &operator=(const TimeStamp &stamp);
  // return 2022-08-26
  std::string ToFormatToday();
  // return 2022-08-26 17:10 45.505045
  std::string ToFormatTodayNowMs();
  std::string ToFormatTodayNowUs();
  std::string ToFormatLogName();

  // return std::time_t
  uint64_t getNowU64() { return sinceepoch_; }

  // 返回一个现在时间戳
  static TimeStamp Now();

private:
  auto GetTimeofDayUs() -> uint64_t;
  auto GetTimeofDayMs() -> uint64_t;
  static constexpr uint64_t kMilliSecondsPerSecond = 1000ull;
  static constexpr uint64_t kMicroSecondsPerSecond = 1000ull * 1000;

  std::time_t sinceepoch_;
  uint64_t time_sec_;
  uint64_t time_usec_;
};

} // namespace shaun