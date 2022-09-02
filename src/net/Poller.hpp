#ifndef SRC_BASE_NET_POLLER_H_
#define SRC_BASE_NET_POLLER_H_

#include <map>
#include <vector>

#include "src/base/NonCopyable.hpp"
#include "src/base/TimeStamp.hpp"
#include "src/base/log/Logging.hpp"
#include <errno.h>

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

  fver::base::TimeStamp UpdateChannelLists(std::vector<Channel*>* channums,
                                      int timeoutms) {
    return static_cast<T*>(this)->UpdateChanelLists(channums, timeoutms);
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

class Poll : public Poller<Poll> {
 public:
  Poll(EventLoop* loop) : loop_(loop) {}

  TimeStamp UpdateChannelLists(std::vector<Channel*>* channums,
                                      int timeoutms) {
    int events_n = ::poll(pollnums_.begin(), pollnums_.size(), timeoutms);
    auto savederror = errno;
    TimeStamp tnow(TimeStamp::Now());
    if (events_n > 0) {
      LOG_TRACE("Poller {} events_n react", events_n);
      for (auto pfd = pollnums_.begin(); pfd != pollnums_.end() && events_n > 0; ++pfd) {
        if (pfd->revents > 0) {
          --events_n;
          auto reventschaniter = channelMap_.find(pfd->fd);
          auto channptr = reventschaniter.second;
          channptr->set_revents(pfd->revents);
          channums->push_back(channptr);
        }
      }
    } else if (events_n == 0) {
      LOG_INFO("Poller No events react");
    } else {
      if (savederror != EINTR) {
        LOG_ERROR("Poller Error!");
      }
    }
    return tnow;
  }

  void RemoveChannel(Channel* channel) {}

  void HasChannel(Channel* channel) {}

 private:
  std::map<int, Channel*> channelMap_;
  EventLoop* loop_;
  std::vector<struct pollfd> pollnums_;
}

}  // namespace fver::net

#endif