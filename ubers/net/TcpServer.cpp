#include "fcntl.h"
#include "TcpServer.h"
#include "EventLoop.h"
#include "EventLoopThread.h"
#include "EventLoopThreadPool.h"
#include "SocketOps.h"
#include <assert.h>
#include <functional>
#include <boost/get_pointer.hpp>



using namespace UBERS;
using namespace UBERS::net;

TcpServer::TcpServer(EventLoop* loop, struct sockaddr_in listenaddr)
  : loop_(loop),
    threadpool_(std::make_unique<EventLoopThreadPool>(loop)),
    started_(false),
    acceptfd_(sockets::CreateStreamNonBlockOrDie()),
    idlefd_(open("dev/null", O_RDONLY | O_CLOEXEC)),
    acceptchannel_(loop, acceptfd_)
{
  sockets::SetReuseAddr(acceptfd_, true);
  sockets::SetReusePort(acceptfd_, true);
  sockets::BindOrDie(acceptfd_,&listenaddr);
  acceptchannel_.SetReadCallBack([this]{ AcceptHandleRead(); });
}

TcpServer::~TcpServer()
{
  loop_->AssertInLoopThread();
  ::close(idlefd_);
}

void TcpServer::SetThreadNums(int n)
{
  assert(n >= 0);
  threadpool_->SetThreadNum(n);
}

void TcpServer::acceptsocketlisten()
{
  sockets::ListenOrDie(acceptfd_);
  acceptchannel_.EnableEvents(kReadEventET);
}

void TcpServer::AcceptHandleRead()
{
  loop_->AssertInLoopThread();
  struct sockaddr_in peeraddr;
  bzero(&peeraddr, sizeof(sockaddr_in));
  int connfd;
  while(true)
  {
    connfd = sockets::Accept(acceptfd_, &peeraddr);
    if(connfd >= 0)
    {
      LOG_INFO << "TcpServer acceptHandleread";
      NewConnection(connfd);
    }
    else
    {
      if(errno == EAGAIN)
      {
        break;
      }
      if(errno == EMFILE)
      {
        LOG_SYSERR << "socket is too many";
        ::close(idlefd_);
        idlefd_ = accept(acceptfd_, nullptr, nullptr);
        ::close(idlefd_);
        idlefd_ = open("/dev/null", O_RDONLY | O_CLOEXEC);
      }
    }
  }
}

void TcpServer::Start()
{
  if(!started_)
  {
    started_ = true;
    threadpool_->Start(threadinitcallback_);
    loop_->RunInLoop([this] { acceptsocketlisten();});
  }
}

void TcpServer::NewConnection(int sockfd)
{
  loop_->AssertInLoopThread();
  EventLoop* nextloop = threadpool_->GetNextLoop(); 
  nextloop->QueueInLoop([nextloop, sockfd, this] { nextloop->CreatConnection(sockfd, conncallback_, messagecallback_, writecomplecallback_);});
}

