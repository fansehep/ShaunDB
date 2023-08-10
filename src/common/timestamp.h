#pragma once

#include <string>
#include <stdint.h>

namespace shaun {


class TimeStamp {
public:
  TimeStamp() = default;
  ~TimeStamp() = default;
  TimeStamp &operator=(const TimeStamp &stamp) = default;
  
  // return 2022-08-26
  auto to_day() -> std::string;
  // return 2022-08-26 17:10 45.505045
  auto to_day_ms() -> std::string;
  auto to_day_us() -> std::string;
  auto to_day_min() -> std::string;

  // return std::time_t
  auto epoch() { return sinceepoch_; }

  // 更新当前时间戳
  auto update() -> void;

private:
  auto time_of_day_us() -> uint64_t;
  auto time_of_day_ms() -> uint64_t;
  static constexpr uint64_t kMilliSecondsPerSecond = 1000ull;
  static constexpr uint64_t kMicroSecondsPerSecond = 1000ull * 1000;

  int64_t sinceepoch_;
  struct timeval tval_;
};

} // namespace shaun