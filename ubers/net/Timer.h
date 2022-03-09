#ifndef _UBERS_NET_TIMER_H_
#define _UBERS_NET_TIMER_H_
#include <boost/noncopyable.hpp>
#include <utility>
#include "CallBack.h"
#include <atomic>
#include "../base/TimeStamp.h"

namespace UBERS::net
{
class Timer : public boost::noncopyable
{
public:
  Timer(TimerCallBack func, TimeStamp timepoint, double interval)
    : callback_(std::move(func)),
      expiration_(timepoint),
      interval_(interval),
      repeat_(interval > 0.0),
      sequence_(Timer::sNumCreated_.fetch_add(1))
      {
      }
  [[nodiscard]] const TimeStamp GetExpiration() { return this->expiration_;}
  [[nodiscard]] bool IsRepeat() { return this->repeat_;}
  void Restart(TimeStamp time);
  int64_t GetSequence() { return this->sequence_;} 
  void Run() const { callback_();}
  bool IsDeleted() { return this->deleted_;}
  void SetDeleted(bool ue) {this->deleted_ = ue;}

  static int64_t GetnumCreated() { return sNumCreated_.load();}

private:
  //todo 定时器回调函数
  const TimerCallBack callback_;
  //todo 下一次超时时刻 
  TimeStamp expiration_;
  //todo 超时时间间隔
  const double interval_;
  //todo 是否重复
  const bool repeat_;
  //* 是否销毁
  bool deleted_ = false;
  //todo 定时器序号
  int64_t sequence_;
  //todo 当前已经创建的定时器数量
  inline static std::atomic<int64_t> sNumCreated_{0};
};
}
#endif