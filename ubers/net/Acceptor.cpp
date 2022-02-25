#include "Acceptor.h"
#include "SocketOps.h"
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "InetAddress.h"
#include "EventLoop.h"
#include "../base/Logging.h"

using namespace UBERS::net;

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport)
  : loop_(loop),
    acceptsocket_(sockets::CreateSocket(listenAddr.family())),
    acceptchannel_(loop, acceptsocket_.GetFd()),
    listening_(false),
    idleFd_(::open("/dev/null", O_RDONLY | O_CLOEXEC))
{
  assert(idleFd_ >= 0);
  acceptsocket_.SetReuseAddr(true);
  acceptsocket_.SetReusePort(reuseport);
  acceptsocket_.BindAddress(listenAddr);
  acceptchannel_.SetReadCallBack([this]{Acceptor::HandleRead();});
}

Acceptor::~Acceptor()
{
  acceptchannel_.DisableAll();
  acceptchannel_.Remove();
  ::close(idleFd_);
}

void Acceptor::SetNewConnectionCallback(const NewConnectionCallback& func)
{
  newconncallback_ = std::move(func);
}

void Acceptor::Listen()
{
  loop_->AssertInLoopThread();
  listening_ = true;
  acceptsocket_.Listen();
  acceptchannel_.EnableEvents(kReadEventLT);
}

void Acceptor::HandleRead()
{
  loop_->AssertInLoopThread();
  InetAddress peeraddr;
  int connfd = acceptsocket_.Accept(&peeraddr);

  if( connfd >= 0)
  {
    if(newconncallback_)
    {
      newconncallback_(connfd, peeraddr);
    }
    else
    {
     ::close(connfd);
    }
  }
  else
  {
    LOG_SYSERR << "Acceptor::HandleRead";
    ::close(idleFd_);
  }  
}