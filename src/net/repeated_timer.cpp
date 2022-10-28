#include "src/net/repeated_timer.hpp"

#include "src/base/log/logging.hpp"
#include "src/net/net_server.hpp"

namespace fver {

namespace net {

void RepeatedTimerCall(int fd, short event, void* arg) {
  auto repeatimer = static_cast<RepeatedTimer*>(arg);
#ifdef DEBUG
  LOG_INFO("repeater timer trigger!");
#endif
  if (repeatimer->callback_) {
    repeatimer->callback_(repeatimer);
    LOG_INFO("retimer {} trigger {}s:{}us", repeatimer->timerName_,
             repeatimer->time_val_.tv_sec, repeatimer->time_val_.tv_usec);
  }
  evtimer_add(&(repeatimer->tick_evt_), &(repeatimer->time_val_));
}

void RepeatedTimer::Reset(const struct timeval& time_val,
                          RepeatedTimerCallback callback,
                          const std::string& name) {
  evtimer_del(&tick_evt_);
  this->time_val_ = time_val;
  this->callback_ = callback;
  this->timerName_ = name;
  Run();
}

RepeatedTimer::RepeatedTimer(const std::shared_ptr<NetServer>& sev,
                             const std::string& timerName)
    : timerName_(timerName), server_(sev), running_(false) {}

void RepeatedTimer::Stop() {
  evtimer_del(&tick_evt_);
  running_ = false;
}

RepeatedTimer::~RepeatedTimer() {
  if (running_ == true) {
    Stop();
  }
}

void RepeatedTimer::Run() {
  assert(server_ != nullptr);
  auto ue = event_assign(&tick_evt_, server_->eventBase_, -1, 0,
                         RepeatedTimerCall, this);
  if (ue == -1) {
    LOG_WARN("retimer {} init fail {} {}", timerName_, time_val_.tv_sec,
             time_val_.tv_usec);
  }
  ue = event_add(&(tick_evt_), &time_val_);
  if (ue == -1) {
    LOG_WARN("retimer {} add fail {} {}", timerName_, time_val_.tv_sec,
             time_val_.tv_usec);
  }
  running_ = true;
}

void RepeatedTimer::Init(const struct timeval& time_val,
                         RepeatedTimerCallback cb,
                         const std::shared_ptr<NetServer>& sev,
                         const std::string& name) {
  this->time_val_ = time_val;
  this->callback_ = cb;
  this->server_ = sev;
  this->timerName_ = name;
  this->running_ = false;
}

}  // namespace net

}  // namespace fver