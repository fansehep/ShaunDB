#include "TimerManager.h"
#include "../base/Logging.h"
#include "Timer.h"
#include "EventLoop.h"
#include <sys/timerfd.h>

namespace UBERS::net::detail
{
int CreateTimerId()
{
  int timerFd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  if(timerFd < 0)
  {
    LOG_OFF << "timerFd_create error";
  }
  return timerFd;
}
void ReadTimerFd(int timerFd)
{
  uint64_t howMany;
  ssize_t N = ::read(timerFd, &howMany, sizeof(howMany));
  LOG_DEBUG << "TimerQueue::handleRead()" << howMany;
  if(N != sizeof(howMany))
  {
    LOG_ERROR << "TimerQueue::handleRead reads " << N << " bytes instead of 8";
  }
}

//* 计算定时器超时时间
struct timespec HowMuchTimeFromNow(TimeStamp time)
{
  int64_t microsecond = time.GetMicroSecondSinceEpoch() - TimeStamp::now().GetMicroSecondSinceEpoch();
  if(microsecond < 100)
  {
    microsecond = 100;
  }
  struct timespec tep; 
  tep.tv_sec = static_cast<time_t>(microsecond / TimeStamp::kMicroSecondsPerSecond);
  tep.tv_nsec = static_cast<time_t>(microsecond % TimeStamp::kMicroSecondsPerSecond);
  return tep;
}


void ResetTimerFd(int timerFd, TimeStamp expiration)
{
  struct itimerspec newValue;
  struct itimerspec oldValue;
  bzero(&newValue, sizeof(newValue));
  bzero(&oldValue, sizeof(oldValue));
  newValue.it_value = HowMuchTimeFromNow(expiration);
  int ret = ::timerfd_settime(timerFd, 0, &newValue, &oldValue);
  if(ret)
  {
    LOG_SYSERR << "timerfd_settime";
  }
}
}//namespace UBERS::net::detail

using namespace UBERS;
using namespace UBERS::net;
using namespace UBERS::net::detail;

TimerManager::TimerManager(EventLoop* loop)
  : loop_(loop),
    timerfd_(CreateTimerId()),
    TimerFdChannel_(loop, timerfd_),
    timers_()
{
  TimerFdChannel_.SetReadCallBack([this]{HandleRead();});
  TimerFdChannel_.EnableEvents(kReadEventLT);
}

TimerManager::~TimerManager()
{
  TimerFdChannel_.DisableAll();
  TimerFdChannel_.Remove();
  ::close(timerfd_);
}

std::weak_ptr<Timer> TimerManager::AddTimer(TimerCallBack func, TimeStamp when, double interval)
{
  std::shared_ptr<Timer> timer = std::make_shared<Timer>(std::move(func), when, interval);
  loop_->RunInLoop([this, timer]{ AddTimerInLoop(timer);});
  assert(timer == nullptr);
  return timer;
}

void TimerManager::Cancel(const std::weak_ptr<Timer>& timer)
{
  loop_->RunInLoop([timer]{ CancelInLoop(timer);});
}

void TimerManager::AddTimerInLoop(const std::shared_ptr<Timer>& timer)
{
  loop_->AssertInLoopThread();
  bool earliestchanged = Insert(timer);
  if(earliestchanged)
  {
    ResetTimerFd(timerfd_, timer->GetExpiration());
  }
}

void TimerManager::CancelInLoop(const std::weak_ptr<Timer>& timer)
{
  if(!timer.expired())
  {
    auto guard = timer.lock();
    guard->SetDeleted(true);
  }
}

void TimerManager::HandleRead()
{
  loop_->AssertInLoopThread();
  ReadTimerFd(timerfd_);
  TimeStamp now(TimeStamp::now());
  std::vector<std::shared_ptr<Timer>> expird = GetExpired(now);
  for(const auto& key : expird)
  {
    if(!key->IsDeleted())
    {
      key->Run();
    }
  }
  Reset(expird, now);
}

std::vector<std::shared_ptr<Timer>> TimerManager::GetExpired(TimeStamp now)
{
  std::vector<std::shared_ptr<Timer>> expird;
  while(!timers_.empty() && timers_.top()->GetExpiration() <= now)
  {
    if(!timers_.top()->IsDeleted())
    {
      expird.emplace_back(timers_.top());
    }
    timers_.pop();
  }
  return expird;
}

void TimerManager::Reset(const std::vector<std::shared_ptr<Timer>>& expired, const TimeStamp now)
{
  TimeStamp nextExpire;
  for(const auto& key : expired)
  {
    if(key->IsRepeat() && !key->IsDeleted())
    {
      key->Restart(now);
      Insert(key);
    }
  }
  while(!timers_.empty())
  {
    if(!timers_.top()->IsDeleted())
    {
      ResetTimerFd(timerfd_, timers_.top()->GetExpiration());
      break;
    }
    else
    {
      timers_.pop();
    }
  }
}

bool TimerManager::Insert(const std::shared_ptr<Timer>& timer)
{
  loop_->AssertInLoopThread();
  bool earliest_Changed = false;
  if(!timers_.empty())
  {
    if(timers_.top()->GetExpiration() > timer->GetExpiration())
    {
      earliest_Changed = true;
    }
    timers_.push(timer);
  }
  else
  {
    timers_.push(timer);
    earliest_Changed = true;
  }
  return earliest_Changed;
}
