#ifndef SRC_NET_SINGLE_TIMER_H_
#define SRC_NET_SINGLE_TIMER_H_
#include "src/net/net_server.hpp"
extern "C" {
#include <event2/event.h>
#include <event2/event_struct.h>
}

#include <cstdint>
#include <functional>

#include "src/base/noncopyable.hpp"

using ::fver::base::NonCopyable;

namespace fver {

namespace net {

class SingleTimer;

using SingleTimerCallback = std::function<int(SingleTimer*)>;

class Connection;

// arg 将会传递 SingleTimer
extern void SingleTimerCall(int fd, short event, void* arg);

// 封装了一次性定时器
class SingleTimer : public NonCopyable {
 public:
  friend NetServer;
  SingleTimer() = default;

  friend void SingleTimerCall(int fd, short event, void* arg);

  SingleTimer(const std::shared_ptr<NetServer>& sev,
              const std::string& timerName);

  ~SingleTimer();

  void Init(const struct timeval& time_val, SingleTimerCallback callback,
            const std::shared_ptr<NetServer>& sev,
            const std::string& timername);

  void Run();

  void Stop();

  void Reset(const struct timeval& time_val, SingleTimerCallback callback,
             const std::string& name);

 private:
  SingleTimerCallback callback_;
  std::string timerName_;
  std::shared_ptr<NetServer> server_;
  bool running_;
  struct event tick_evt_;
  struct timeval time_val_;
};

}  // namespace net

}  // namespace fver

#endif