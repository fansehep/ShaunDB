#ifndef SRC_NET_POLL_H_
#define SRC_NET_POLL_H_

#include <map>
#include <vector>

#include "src/base/TimeStamp.hpp"
#include "src/base/log/Logging.hpp"
#include "poll.h"


namespace fver::net {

class Channel;
class EventLoop;

class Poll : public Poller<Poll> {
 public:
  Poll(EventLoop* loop) : loop_(loop) {}

  TimeStamp PollChannelLists(std::vector<Channel*>* channums, int timeoutms) {
    int events_n = ::poll(pollnums_.begin(), pollnums_.size(), timeoutms);
    auto savederror = errno;
    TimeStamp tnow(TimeStamp::Now());
    if (events_n > 0) {
      LOG_TRACE("Poller {} events_n react", events_n);
      for (auto pfd = pollnums_.begin(); pfd != pollnums_.end() && events_n > 0;
           ++pfd) {
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

  void UpdateChannel(Channel* channel) {
    LOG_INFO("fd = {} events = {}", channel->GetFd(), channel->GetEvents());
    // 新的 channel 加入到 Poll数组中
    if (channel->GetPollIndex() < 0) {
      struct pollfd newpollfd;
      newpollfd.fd = channel->GetFd();
      newpollfd.events = static_cast<short>(channel->GetEvents());
      newpollfd.revents = 0;
      pollnums_.push_back(newpollfd);
      int channelindex = static_cast<int>(pollnums_.size()) - 1;
      channel->SetPollIndex(channelindex);
      channelMap_[newpollfd.fd] = channel;
    } else {
      int index = channel->GetIndex();
      auto& pfdref = pollnums_[index];
      pfdref.fd = channel.fd();
      pfdref.events = static_cast<short>(channel->GetEvents());
      pfdref.revents = 0;
      if (channel->IsNoneEvents()) {
        pfdref.fd = -channel->GetFd() - 1;
      }
    }
  }

  void RemoveChannel(Channel* channel) {
    LOG_INFO("fd = {} RemoveChannel", channel->GetFd());
    auto n = channelMap_.erase(channel->GetFd());
    auto idx = channel->GetPollIndex();
    if (n == (pollnums_.size() - 1)) {
      pollnums_.pop_back();
    } else {
      // 要删除中间的某一个 vector 元素
      // 将最后一个元素与需要被删除元素交换, 再删除
      int pollnumsendfd = pollnums_.back().fd;
      std::iter_swap(pollnums_.begin() + idx, pollnums_.end() - 1);
      if (pollnumsendfd < 0) {
        // -1 -> 0
        pollnumsendfd = -pollnumsendfd - 1;
      }
      channelMap_[pollnumsendfd]->SetPollIndex(idx);
      pollnums_.pop_back();
    }
  }

  void HasChannel(Channel* channel) {
    auto chaniter = channelMap_.find(channel->GetFd());
    return chaniter != channelMap_.end() && chaniter->second == channel;
  }

 private:
  std::map<int, Channel*> channelMap_;
  EventLoop* loop_;
  std::vector<struct pollfd> pollnums_;
}

}  // namespace fver::net

#endif