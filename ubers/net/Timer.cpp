#include "Timer.h"

namespace UBERS::net
{

  void Timer::Restart(TimeStamp time)
  {
    if(repeat_)
    {
      expiration_ = addTime(time, interval_);
    }
    else
    {
      expiration_ = UBERS::TimeStamp();
    }
  }
}