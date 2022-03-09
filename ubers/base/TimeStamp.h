/* 
* 主要提供日志的时间记录，和辅助定时器
*/

#ifndef _UBERS_TIMESTAMP_H_
#define _UBERS_TIMESTAMP_H_
#include <chrono>
#include <string.h>
#include <string>
#include <boost/operators.hpp>
namespace UBERS
{
/*
* boost::equality_comparable<> 继承此类 自己实现 operator < 他帮你完成 operator > >= <=
* boost::less_than_copmarable<> 继承此类 自己实现 operator == 该类帮你完成 !=
*/
class TimeStamp : public boost::equality_comparable<TimeStamp>,
                  public boost::less_than_comparable<TimeStamp>
{
public:
  TimeStamp(std::time_t newTime) : MicroSecondSinceEpoch_(newTime){}
  TimeStamp() {}
  ~TimeStamp() {}

  std::time_t GetMicroSecondSinceEpoch() const { return this->MicroSecondSinceEpoch_; }

  std::string ToString() const;

  //* 是否展示具体的天数后面的日期 2022 02 26 
  std::string ToFormattedString(bool showToday = false);
  //* 返回当前时间 时间戳
  static TimeStamp now();

  static const int kMicroSecondsPerSecond = 1000 * 1000;

  void swap(TimeStamp time) { this->MicroSecondSinceEpoch_ = time.MicroSecondSinceEpoch_;}
private:
  std::time_t MicroSecondSinceEpoch_ = 0;
};

inline bool operator < (TimeStamp left, TimeStamp right)
{
  return left.GetMicroSecondSinceEpoch() < right.GetMicroSecondSinceEpoch();
}

inline bool operator == (TimeStamp left, TimeStamp right)
{
  return left.GetMicroSecondSinceEpoch() == right.GetMicroSecondSinceEpoch();
}

inline double timeDifference(TimeStamp left, TimeStamp right)
{
  int64_t difference = left.GetMicroSecondSinceEpoch() - right.GetMicroSecondSinceEpoch();
  return static_cast<double>(difference) / TimeStamp::kMicroSecondsPerSecond;
}

//* 
inline TimeStamp addTime(TimeStamp target, double seconds)
{
  int64_t delta = static_cast<int64_t>(seconds *TimeStamp::kMicroSecondsPerSecond);
  return TimeStamp(target.GetMicroSecondSinceEpoch() + delta);
}

}
#endif