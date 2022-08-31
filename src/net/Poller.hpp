#ifndef SRC_BASE_NET_POLLER_H_
#define SRC_BASE_NET_POLLER_H_

#include "src/base/NonCopyable.hpp"
#include "src/base/log/Logging.hpp"
#include "src/base/TimeStamp.hpp"

namespace fver::net {

/*
1. int epoll_wait(int epfd, struct epoll_event* events,
       int maxevents, int timeout);
   int epoll_pwait(int epfd, struct epoll_event* events,
       int maxevents, int timeout, const sigset_t* sigmask);
   int epoll_create(int size); x
   int epoll_create1(int flags); ok
   int epoll_ctl(int epfd, int op, int fd, struct epoll_event* event);
2. 
*/


template <typename T>
class Poller : public fver::base::NonCopyable {
 public:
  Poller(EventLoop* loop);
  ~Poller();
  template <typename T>
  int UpdateChannel()
 private:
};

}  // namespace fver::net

#endif