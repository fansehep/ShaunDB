#include <fcntl.h>
#include <boost/get_pointer.hpp>
#include <assert.h>
#include "TcpServer.h"
#include "EventLoop.h"
#include "EventLoopThread.h"
#include "EventLoopThreadPool.h"
#include "SocketOps.h"
#include "Channel.h"

using namespace UBERS;
using namespace UBERS::net;

TcpServer::TcpServer(EventLoop* loop, struct sockaddr_in ListenAddr)
  : loop_(loop),
    threadpool_(std::make_unique<EventLoopThreadPool>(loop)),
    connection_callback_(DefaultConnectionCallBack),
    message_callback_(DefaultMessageCallBack),
    acceptFd_(socketops::CreateStreamNonBlockOrDie()),
    IdleFd_(::open("/dev/null", O_RDONLY | O_CLOEXEC)),
    acceptChannel_(Channel(loop, acceptFd_))
{
  socketops::SetReuseAddr(acceptFd_, true);
  socketops::SetReusePort(acceptFd_, true);
  socketops::BindOrDie(acceptFd_, &ListenAddr);
  acceptChannel_.SetReadCallBack([this]{ AcceptHandRead();});
}

TcpServer::~TcpServer()
{ 
  loop_->AssertInLoopThread();
  ::close(IdleFd_);
}

void TcpServer::SetThreadNum(int n)
{
  assert(n > 0);
  threadpool_->SetThreadNum(n);
}

void TcpServer::AcceptSocketListen()
{
  socketops::ListenOrDie(acceptFd_);
  acceptChannel_.EnableEvents(kReadEventET);
}

void TcpServer::AcceptHandRead()
{
  loop_->AssertInLoopThread();
  struct sockaddr_in Peer_addr;
  bzero(&Peer_addr, sizeof(Peer_addr));
  int conn_fd;
  while(1)
  {
    conn_fd = socketops::Accept(acceptFd_, &Peer_addr);
    if(conn_fd >= 0)
    {
      LOG_DEBUG << "accept success" << inet_ntoa(Peer_addr.sin_addr);
      //FIXME
    }
  }
}