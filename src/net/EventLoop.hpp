#ifndef SRC_NET_EVENTLOOP_H_
#define SRC_NET_EVENTLOOP_H_

#include <atomic>
#include <functional>
#include <vector>


namespace fver::net {


class Channel;
class Poller;


class EventLoop : public fver::base::NonCopyable {
public:
  using func = std::function<void()>;
  EventLoop();
  ~EventLoop();
  void Loop();
  void Quit();
};

}


#endif