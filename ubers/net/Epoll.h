#ifndef _UBERS_EPOLL_H_
#define _UBERS_EPOLL_H_
#include <vector>
#include <queue>
#include <map>
#include <string>
#include <boost/noncopyable.hpp>
#include "EventLoop.h"
#include <unordered_map>
#include "CallBack.h"

struct epoll_event;
namespace UBERS::net
{
class Channel;
class TcpConnection;

class Epoll : public boost::noncopyable
{
public:
  using ChannelVec = std::vector<Channel*>;
  explicit Epoll(EventLoop* loop);
  ~Epoll();
  void UpdateChannel(Channel* channel);
  void RemoveChannel(Channel* channel);
  void Poll(ChannelVec* activeChannels);
  void CreateConnection(int Sockfd, const ConnectionCallBack& connectioncallback, \
   const MessageCallBack& messagecallback, const WriteCompleteCallBack& writecallcompleteback);

private:
  enum{ kNew = -1, kAdded = 1, kDeleted = 2};

  static const int kInitEventSize = 16;
  void RemoveConnection(const TcpConnectionPtr& connm);
  void FillActiveChannels(int numEvents, ChannelVec* activeChannels) const;
  void Update(int operation, Channel* channel) const;

  using ChannelMap = std::unordered_map<int, Channel*>;
  using TcpConnMap = std::unordered_map<int, TcpConnectionPtr>;

  std::vector<TcpConnectionPtr> connectionsPool_;
  ChannelMap channels_;
  TcpConnMap connections_;
  int epollfd_;
  std::vector<struct epoll_event> events_;
  EventLoop* ownerLoop_;
};


}
#endif