#include "src/net/single_timer.hpp"

#include "src/base/log/logging.hpp"
#include "src/net/net_server.hpp"

namespace fver {

namespace net {

void SingleTimerCall(int fd, short event, void *arg) {
  auto single_timer = static_cast<SingleTimer *>(arg);
#ifdef DEBUG
  LOG_INFO("single timer trigger!");
#endif
  if (single_timer->callback_) {
    single_timer->callback_(single_timer);
  }
  LOG_INFO("sitimer: {} trigger {}s:{}us", single_timer->timerName_,
           single_timer->time_val_.tv_sec, single_timer->time_val_.tv_usec);
  single_timer->Stop();
  single_timer->running_ = false;
}

SingleTimer::SingleTimer(const std::shared_ptr<NetServer> &sev,
                         const std::string &name)
    : timerName_(name), server_(sev) {}

void SingleTimer::Reset(const struct timeval &time_val,
                        SingleTimerCallback callback, const std::string &name) {
  evtimer_del(&tick_evt_);
  this->time_val_ = time_val;
  this->callback_ = callback;
  Run();
}

void SingleTimer::Stop() {
  if (running_ == true) {
    evtimer_del(&tick_evt_);
  }
  LOG_WARN("sitimer: {} stop {} {}", this->timerName_, this->time_val_.tv_sec,
           this->time_val_.tv_usec);
  running_ = false;
}

void SingleTimer::Run() {
  assert(server_ != nullptr);
  auto ue = event_assign(&tick_evt_, server_->eventBase_, -1, 0,
                         SingleTimerCall, this);
  if (ue == -1) {
    LOG_WARN("sitimer {} init fail {} {}", timerName_, time_val_.tv_sec,
             time_val_.tv_usec);
  }
  ue = event_add(&(tick_evt_), &time_val_);
  if (ue == -1) {
    LOG_WARN("sitimer {} add fail {} {}", timerName_, time_val_.tv_sec,
             time_val_.tv_usec);
  }
  running_ = true;
}

SingleTimer::~SingleTimer() { Stop(); }

void SingleTimer::Init(const struct timeval &time_val,
                       SingleTimerCallback callback,
                       const std::shared_ptr<NetServer> &sev,
                       const std::string &timername) {
  this->time_val_ = time_val;
  this->callback_ = callback;
  this->server_ = sev;
  this->timerName_ = timername;
  this->running_ = false;
}

}  // namespace net

}  // namespace fver