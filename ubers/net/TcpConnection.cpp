#include "TcpConnetion.h"
#include "Channel.h"
#include "EventLoop.h"
#include <string>
using namespace UBERS;
using namespace UBERS::net;


 TcpConnection::TcpConnection(EventLoop* loop, int Sockfd)
  : loop_(loop),
    socketfd_(Sockfd),
    channel_(std::make_unique<Channel>(loop, Sockfd)),
    reading_(true),
    state_(kConnecting)
{
  channel_->SetReadCallBack( [this]{HandleRead();});
  channel_->SetWriteCallBack( [this]{HandleWrite();});
  channel_->SetCloseCallBack( [this]{HandleClose();});
  channel_->SetErrorCallBack( [this]{HandleError();});
  sockets::SetKeepAlive(socketfd_, true);
}    

TcpConnection::~TcpConnection()
{
  assert(state_ == kDisconnecting);
}

void TcpConnection::Send(std::string_view message)
{
  if(state_ == kConnected)
  {
    if(loop_->isInLoopThread())
    {
      SendInLoop(message.data(), message.size());
    }
  }
  else
  {
    loop_->RunInLoop([this, &message] {SendInLoop(message.data(), message.size());});
  }
}

void TcpConnection::Send(const char* data, size_t len)
{
  if(state_ == kConnected)
  {
    if(loop_->isInLoopThread())
    {
      SendInLoop(data, len);
    }
  }
  else
  {
    loop_->RunInLoop([this, &data, &len] {SendInLoop(data, len);});
  }
}

void TcpConnection::SendInLoop(const void* data, size_t len)
{
  loop_->AssertInLoopThread();
  ssize_t nwrote = 0;
  size_t remaining = len;
  bool faulterror = false;
  if(state_ == kDisconnected)
  {
    LOG_WARN << "disconnected, give up writing";
    return;
  }
  if(!channel_->IsWriting() && outputbuffer_.readableBytes() == 0)
  {
    nwrote = sockets::Write(channel_->GetFd(), data, len);
    if(nwrote >= 0)
    {
      remaining = len - nwrote;
      LOG_DEBUG << "sendinloop";
      if(remaining == 0 && writecompcallback_)
      {
        loop_->QueueInLoop([shptr = shared_from_this()]{shptr->writecompcallback_(shptr);});
      }
    }
    else
    {
      nwrote = 0;
      if(errno != EWOULDBLOCK)
      {
        LOG_SYSERR << "TcpConnection::SendInloop";
        if(errno == EPIPE || errno == ECONNRESET)
        {
          faulterror = true;
        }
      }
    }
  }

  assert(remaining <= len);

  if(!faulterror && remaining > 0)
  {
    LOG_DEBUG << "issendfile";
    if(!channel_->IsWriting())
    {
      channel_->EnableEvents(kWriteEvent);
    }
  }
}

void TcpConnection::Shutdown()
{

  if(state_ == kConnected)
  {
    this->state_ = kDisconnecting;
    loop_->RunInLoop([ptr = shared_from_this()]{ptr->ShutDownInLoop();});
  }
}

void TcpConnection::ShutDownInLoop()
{
  loop_->AssertInLoopThread();
  if(!channel_->IsWriting())
  {
    sockets::ShutDownWrite(socketfd_);
  }
}

void TcpConnection::ForceClose()
{
  if(state_ == kConnected || state_ == kDisconnecting)
  {
    this->state_ = kDisconnected;
    loop_->QueueInLoop([ptr = shared_from_this()]{ptr->ForceCloseInLoop();});
  }
}

void TcpConnection::ForceCloseInLoop()
{
  loop_->AssertInLoopThread();
  if(state_ == kConnected || state_ == kDisconnecting)
  {
    HandleClose();
  }
}

void TcpConnection::SetTcpNagle(bool ue)
{
  sockets::SetTcpNoDelay(this->socketfd_, ue);
}

void TcpConnection::Startread()
{
  loop_->RunInLoop([this]{ StartReadInLoop();});
}

void TcpConnection::StartReadInLoop()
{
  loop_->AssertInLoopThread();
  if(!reading_ || !channel_->IsReading())
  {
    channel_->EnableEvents(kReadEventLT);
    reading_ = true;
  }
}

void TcpConnection::Stopread()
{
  loop_->RunInLoop([this]{StopReadInLoop();});
}

void TcpConnection::StopReadInLoop()
{
  loop_->AssertInLoopThread();
  if(reading_ || channel_->IsReading())
  {
    channel_->DisableEvents(kReadEventLT);
    reading_ = false;
  }
}

void TcpConnection::ConnectEstablished()
{
  loop_->AssertInLoopThread();
  assert(state_ == kConnected);
  state_ = kConnected;
  channel_->EnableEvents(kReadEventLT);

  conncallback_(shared_from_this());
}

void TcpConnection::ConnectDestroyed()
{
  loop_->AssertInLoopThread();
  if(state_ == kConnected)
  {
    state_ = kDisconnected;
    channel_->DisableAll();
    conncallback_(shared_from_this());
  }
  channel_->Remove();
}

void TcpConnection::HandleRead()
{
  LOG_DEBUG << "handleread";
  loop_->AssertInLoopThread();
  int saveerrno = 0;
  ssize_t n = inputbuffer_.readFd(channel_->GetFd(), &saveerrno);
  if(n > 0)
  {
    messcallback_(shared_from_this(), &inputbuffer_);
  }
  else if(n == 0)
  {
    HandleClose();
  }
  else
  {
    errno = saveerrno;
    LOG_SYSERR << "TcpConnection::handleread";
    HandleError();
  }
}

void TcpConnection::HandleError()
{
  int err = sockets::GetSocketError(channel_->GetFd());
  LOG_ERROR << "Tcpconnection " << err;
}

void TcpConnection::HandleWrite()
{
  loop_->AssertInLoopThread();
  if(channel_->IsWriting())
  {
    ssize_t n = sockets::Write(channel_->GetFd(), outputbuffer_.peek(), outputbuffer_.readableBytes());
    if(n > 0)
    {
      outputbuffer_.retrieve(n);
      if(outputbuffer_.readableBytes() == 0)
      {
        channel_->DisableEvents(kWriteEvent);
        if(writecompcallback_)
        {
          loop_->QueueInLoop(std::bind(writecompcallback_, shared_from_this()));
        }
        if(state_ == kDisconnecting)
        {
          ShutDownInLoop();
        }
      }
    }
    else
    {
      LOG_SYSERR << "Tcpconnection::handlewrite";
    }
  }
  else
  {
     LOG_FATAL  << "Connection fd = " << channel_->GetFd() << "is down";
  }
}

void TcpConnection::HandleClose()
{
  loop_->AssertInLoopThread();
  assert(state_ == kConnected || state_ == kDisconnecting);
  
  state_ = kDisconnected;
  channel_->DisableAll();

  TcpConnectionPtr guardptr(shared_from_this());
  conncallback_(guardptr);
  closecallback_(guardptr);
}

void TcpConnection::ConnectReset(int fd)
{
  state_ = kConnecting;
  socketfd_ = fd;
  channel_->ChannelReset(fd);
  reading_ = true;
  inputbuffer_.retrieveAll();
  outputbuffer_.retrieveAll();
  sockets::SetKeepAlive(socketfd_, true);
}