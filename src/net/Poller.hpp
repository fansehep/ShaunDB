#ifndef SRC_BASE_NET_POLLER_H_
#define SRC_BASE_NET_POLLER_H_

#include <errno.h>

#include <map>
#include <vector>

#include "src/base/NonCopyable.hpp"


using fver::base::TimeStamp;

namespace fver::net {

/*
1.
   struct epoll_event {
    uint32_t events;
    struct epoll_data {
      void* ptr;
      int fd;
      uint32_t u32;
      uint64_t u64;
    };
   };
   int epoll_wait(int epfd, struct epoll_event* events,
       int maxevents, int timeout);
   int epoll_pwait(int epfd, struct epoll_event* events,
       int maxevents, int timeout, const sigset_t* sigmask);
   int epoll_create(int size); x
   int epoll_create1(int flags); ok
   int epoll_ctl(int epfd, int op, int fd, struct epoll_event* event);
2. struct pollfd {
    int fd;
    short events;
    short revents;
   };
   poll(struct pollfd* fds, nfds_t nfds, int timeoutms);
*/
class EventLoop;
class Channel;

template <typename T>
class Poller : public fver::base::NonCopyable {
 public:
  Poller() = default;
  ~Poller() = default;

  TimeStamp PollChannelLists(std::vector<Channel*>* channums, int timeoutms) {
    return static_cast<T*>(this)->PollChanelLists(channums, timeoutms);
  }

  void UpdateChannel(Channel* channel) {
    return static_cast<T*>(this)->UpdateChannel(channel);
  }

  void RemoveChannel(Channel* channel) {
    return static_cast<T*>(this)->RemoveChannel(channel);
  }

  void HasChannel(Channel* channel) {
    return static_cast<T*>(this)->HasChannel(channel);
  }
};

}  // namespace fver::net

#endif