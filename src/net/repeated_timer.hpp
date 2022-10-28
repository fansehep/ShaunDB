#ifndef SRC_NET_REPEATED_TIMER_H_
#define SRC_NET_REPEATED_TIMER_H_

extern "C" {
#include <event2/event.h>
#include <event2/event_struct.h>
}
#include <cstdint>
#include <functional>
#include <memory>
#include <string>

#include "src/base/noncopyable.hpp"

using ::fver::base::NonCopyable;

namespace fver {

namespace net {

class Connection;
class RepeatedTimer;
using RepeatedTimerCallback =
    std::function<int(RepeatedTimer*)>;

// arg 将会传递 RepeatedTimer.
extern void RepeatedTimerCall(int fd, short event, void* arg);

class NetServer;

// 对重复定时器的封装
class RepeatedTimer : public NonCopyable {
 public:
  friend NetServer;
  RepeatedTimer() = default;

  friend void RepeatedTimerCall(int fd, short event, void* arg);

  RepeatedTimer(const std::shared_ptr<NetServer>& sev,
                const std::string& timerName);

  ~RepeatedTimer();

  void Init(const struct timeval& time_val, RepeatedTimerCallback callback,
            const std::shared_ptr<NetServer>& sev, const std::string& name);

  void Run();

  void Stop();

  void Reset(const struct timeval& time_val, RepeatedTimerCallback callback,
             const std::string& name);

 private:
  // 定时器的名称
  std::string timerName_;
  // 重复定时器共享NetServer 的所有权
  std::shared_ptr<NetServer> server_;
  // 定时器是否正在运行
  bool running_;
  // 定时器所对应的 event 事件
  struct event tick_evt_;
  // 定时器间隔时间触发
  struct timeval time_val_;
  // 定时器所对应的回调函数
  RepeatedTimerCallback callback_;
};

}  // namespace net

}  // namespace fver

#endif