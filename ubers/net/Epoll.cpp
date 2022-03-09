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
    ownerLoop_(loop),
    connections_()
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
  LOG_INFO << " poll " << "channels.size() = " << channels_.size();
  int numEvents = ::epoll_pwait(epollfd_, &*events_.begin(), static_cast<int>(events_.size()), 1, nullptr);
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
    activeChannels->push_back(channel);
  }
}

void Epoll::UpdateChannel(Channel* channel)
{
  ownerLoop_->AssertInLoopThread();
  const int t_status = channel->status();
  LOG_INFO << "fd = " << channel->GetFd() << "events = " << channel->GetEvent() << "states = " << t_status;
  //* 当前的Channel 还没有被关注或者已经被杀掉了
  if(t_status == kNew || t_status == kDeleted)
  {

    int fd = channel->GetFd();
    //* 新的 Channel 会设置为 kNew
    if(t_status == kNew)
    {
      channels_[fd] = channel;
    }
    //* 设置该channel的状态
    channel->SetStatus(kAdded);
    //* 对该channel进行设置，方便后续的
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
  //* 利用 epoll_data 保存自己所对应的Channel对象，方
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
      LOG_SYSERR << "epoll_ctl op = " << operation << " fd = " << fd;
    }
  }
}

void Epoll::RemoveChannel(Channel* channel)
{
  ownerLoop_->AssertInLoopThread();
  int fd = channel->GetFd();
  int status = channel->status();
  connectionspool_.emplace_back(std::move(connections_[fd]));
  ::close(fd);
  
  if(status == kAdded)
  {
    Update(EPOLL_CTL_DEL, channel);
  }
  channel->SetStatus(kNew);
}
void Epoll:: CreateConnection(int sockFd, const ConnectionCallBack& conncallback, const MessageCallBack& messagecallback, const WriteCompleteCallBack& writecompletecallback)
{
  if(!connectionspool_.empty())
  {
    connectionspool_.back()->ConnectReset(sockFd);
    connections_[sockFd] = std::move(connectionspool_.back());
    connectionspool_.pop_back();
    ownerLoop_->RunInLoop([&conn = connections_[sockFd]] { conn->ConnectEstablished();});
  }
  else
  {
    auto conn = std::make_shared<TcpConnection>(ownerLoop_, sockFd);
    conn->SetConnectionCallBack(conncallback);
    conn->SetMessageCallBack(messagecallback);
    conn->SetWriteCompleteCallBack(writecompletecallback);
    conn->SetCloseCallBack([this](auto && PH1){ RemoveConnection(PH1);});
    connections_[sockFd] = std::move(conn);
    ownerLoop_->RunInLoop([&conn = connections_[sockFd]] { conn->ConnectEstablished();});
  }
}


//FIXME
void Epoll::RemoveConnection( const TcpConnectionPtr& conn)
{
  ownerLoop_->RunInLoop([&conn]{conn->ConnectDestroyed();});
}

