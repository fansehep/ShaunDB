#include "TimeStamp.h"


using namespace UBERS;


std::string TimeStamp::ToString() const
{
  char buf[32] = {0};
  int64_t seconds = MicroSecondSinceEpoch_ / kMicroSecondsPerSecond;
  int64_t microseconds = MicroSecondSinceEpoch_ % kMicroSecondsPerSecond;
  snprintf(buf, sizeof(buf), "%ld %ld", seconds, microseconds);
  return buf;
}

std::string TimeStamp::ToFormattedString(bool ShowToday)
{
  char buf[64] = {0};
  int64_t milli = MicroSecondSinceEpoch_ + static_cast<int64_t>(8 * 60 * 60 * 1000);
  auto mTime = std::chrono::milliseconds(milli);
  auto dpt = std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>(mTime);
  auto tt = std::chrono::system_clock::to_time_t(dpt);
  struct tm now;
  gmtime_r(&tt, &now);
  int64_t MicroSecond = static_cast<int64_t>(MicroSecondSinceEpoch_ % kMicroSecondsPerSecond);
  if(!ShowToday)
  {
    snprintf(buf, sizeof(buf), "%04d %02d %02d %02d:%02d:%02d.%06ld", now.tm_year + 1900, now.tm_mon + 1, now.tm_mday, now.tm_hour, now.tm_min, now.tm_sec, MicroSecond);
  }
  else
  {
    snprintf(buf, sizeof(buf), "%04d %02d %02d", now.tm_year + 1900, now.tm_mon + 1, now.tm_mday);
  }
  return buf;
  }

TimeStamp TimeStamp::now()
{
  std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::microseconds> tp =  \
  std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
  std::time_t temp = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()).count();
  return TimeStamp(temp);
}



