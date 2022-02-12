#include <errno.h>
#include <sys/epoll.h>
#include "Epoll.h"
#include "Channel.h"
#include "TcpConnetion.h"

using namespace UBERS;
using namespace UBERS::net;

Epoll::Epoll(EventLoop* loop)
  : epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
    events_(kInitEventSize),
    ownerLoop_(loop)
{
  if(epollfd_ < 0)
  {
    LOG_ERROR << "epoll::epoll_create";
  }
}

Epoll::~Epoll()
{
  for(auto& item : connections_)
  {
    item.second.reset();
  }
  ::close(epollfd_);
}

void Epoll::Poll(ChannelVec* activeChannels)
{
  int numEvents = ::epoll_wait(epollfd_, &*events_.begin(), \
            static_cast<int>(events_.size()), -1);
  int savedErrno = errno;
  if(numEvents > 0)
  {
    LOG_DEBUG << numEvents << "events happended";
    FillActiveChannels(numEvents, activeChannels);
    if(static_cast<size_t>(numEvents) == events_.size())
    {
      events_.resize(events_.size() * 2);
    }
  }
  else
  {
    if(savedErrno != EINTR)
    {
      errno = savedErrno;
      LOG_ERROR << "epoll::poll";
    }
  }
}          

void Epoll::FillActiveChannels(int numEvents, ChannelVec* activeChannels) const
{ 
  for(int i = 0; i < numEvents; ++i)
  {
    auto *channel = static_cast<Channel*>(events_[i].data.ptr);
    channel->SetRevents(events_[i].events);
    activeChannels->emplace_back(channel);
  }
}

void Epoll::UpdateChannel(Channel* channel)
{
  ownerLoop_->AssertInLoopThread();
  const int t_status = channel->status();
  if(t_status == kNew || t_status == kDeleted)
  {
    int fd = channel->GetFd();
    if(t_status == kNew)
    {
      channels_[fd] = channel;
    }
    channel->SetStatus(kAdded);
    Update(EPOLL_CTL_ADD, channel);
  }
  else
  {
    if(channel->IsNoneEvent())
    {
      Update(EPOLL_CTL_DEL, channel);
      channel->SetStatus(kDeleted);
    }
    else
    {
      Update(EPOLL_CTL_MOD, channel);
    }
  }
}

void Epoll::Update(int operation, Channel* channel) const
{
  struct epoll_event event;
  bzero(&event, sizeof(event));
  event.events = channel->GetEvent();
  event.data.ptr = channel;
  int fd = channel->GetFd();
  if(::epoll_ctl(epollfd_, operation, fd, &event) < 0)
  {
    if(operation == EPOLL_CTL_DEL)
    {
      //* 添加失败，不退出程序
      LOG_ERROR << "epoll_ctl op = " << operation << " fd = " << fd;
    }
    else
    {
      //* 程序挂掉了
      LOG_OFF << "epoll_ctl op = " << operation << " fd = " << fd;
    }
  }
}

//FIXME
void Epoll::CreateConnection(int Sockfd, const ConnectionCallBack& connectioncallback, \
   const MessageCallBack& messagecallback, const WriteCompleteCallBack& writecallcompleteback)
{
  if(!connectionsPool_.empty())
  {
    ;

  }
}

