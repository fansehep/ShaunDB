#ifndef _UBERS_NET_TIMERMANAGER_H_
#define _UBERS_NET_TIMERMANAGER_H_
#include <boost/noncopyable.hpp>
#include <queue>
#include <vector>
#include <memory>
#include "Timer.h"
#include "Channel.h"
namespace UBERS::net
{
class EventLoop;
struct TimerCmp
{
  bool operator()(const std::shared_ptr<Timer>& a, const std::shared_ptr<Timer>& b)
  {
    if(a->GetExpiration() == b->GetExpiration())
    {
      return a->GetSequence() > b->GetSequence();
    }
    else
    {
      return a->GetExpiration() > b->GetExpiration();
    }
  }
};

class TimerManager : public boost::noncopyable
{
public:
  explicit TimerManager(EventLoop* loop);
  ~TimerManager();

  std::weak_ptr<Timer> AddTimer(TimerCallBack func, TimeStamp when, double interval);
  void Cancel(const std::weak_ptr<Timer>& timer);
private:
  void AddTimerInLoop(const std::shared_ptr<Timer>& timer);
  static void CancelInLoop(const std::weak_ptr<Timer>& timer);
  void HandleRead();
  std::vector<std::shared_ptr<Timer>> GetExpired(TimeStamp now);
  void Reset(const std::vector<std::shared_ptr<Timer>>& expired, const TimeStamp now);
  bool Insert(const std::shared_ptr<Timer>& timer);

  EventLoop* loop_;
  const int timerfd_;
  Channel TimerFdChannel_;

  //* 优先队列
  std::priority_queue<std::shared_ptr<Timer>, std::vector<std::shared_ptr<Timer>>, TimerCmp> timers_;
};
}
#endif