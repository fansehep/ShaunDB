#include "EventLoop.h"
#include "Channel.h"
#include "TcpConnetion.h"
#include <sys/epoll.h>

using namespace UBERS;
using namespace UBERS::net;

namespace UBERS::net
{
const unsigned kNoneEvent = 0;;
const unsigned kReadEventLT = EPOLLIN | EPOLLPRI;
const unsigned kReadEventET = EPOLLIN | EPOLLPRI | EPOLLET;
const unsigned kWriteEvent = EPOLLOUT;
}

Channel::Channel(EventLoop* loop, int fd)
  : loop_(loop), fd_(fd)
{
}

Channel::~Channel()
{
  assert(!eventHandling_);
  assert(!addedToLoop_);
  close(fd_);
}

void Channel::Update()
{
  this->addedToLoop_ = true;
  loop_->UpdateChannel(this);
}

void Channel::Remove()
{
  assert(IsNoneEvent());
  addedToLoop_ = false;
  loop_->RemoveChannel(this);
}

void Channel::HandleEvent()
{
  eventHandling_ = true;
  if((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN))
  {
    if(closecallback_) {closecallback_();}
  }
  if(revents_ & EPOLLERR)
  {
    if(errorcallback_) {errorcallback_();}
  }
  if(revents_ & (EPOLLIN | EPOLLPRI | EPOLLHUP))
  {
    if(readcallback_) {readcallback_();}
  }
  if(revents_ & EPOLLOUT)
  {
    if(writecallback_) {writecallback_();}
  }
}

void Channel::ChannelReset(int SocketId)
{
  fd_ = SocketId;
  events_ = 0;
  revents_ = 0;
  status_ = -1;
  eventHandling_ = false;
  addedToLoop_ = false;
}

void Channel::SetReadCallBack(EventCallback func)
{
  this->readcallback_ = std::move(func);
}
void Channel::SetWriteCallBack(EventCallback func)
{
  this->writecallback_ = std::move(func);
}
void Channel::SetCloseCallBack(EventCallback func)
{
  this->closecallback_ = std::move(func);
}
void Channel::SetErrorCallBack(EventCallback func)
{
  this->errorcallback_ = std::move(func);
}
