#include <sys/uio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <netinet/tcp.h>
#include <sys/sendfile.h>
#include "SocketOps.h"
#include "../base/Logging.h"

using namespace UBERS::net;

inline int socketops::CreateStreamNonBlockOrDie()
{
  int Sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
  if(Sockfd < 0)
  {
    LOG_OFF << "sockets::CreateStreamNonBlockOrDie";
  }
  return Sockfd;
}

inline int socketops::CreateDgramNonBlockingOrDie()
{
  int Sockfd = ::socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_UDP);
  if(Sockfd < 0)
  {
    LOG_OFF << "sockets::CreateDgramNonBlockingOrDie";
  }
  return Sockfd;
}

inline int socketops::Connect(int ClientSockfd, struct sockaddr_in* addr)
{
  return ::connect(ClientSockfd, reinterpret_cast<struct sockaddr*>(addr), static_cast<socklen_t>(sizeof(struct sockaddr_in)));
}

inline void socketops::BindOrDie(int ClientSockfd, struct sockaddr_in* addr)
{
  int Returnerrno = ::bind(ClientSockfd, reinterpret_cast<struct sockaddr*>(addr), static_cast<socklen_t>(sizeof(struct sockaddr_in)));
  if(Returnerrno < 0)
  {
    LOG_OFF << "sockets::Bind";
  }
}

inline void socketops::ListenOrDie(int SockFd)
{
  int Returnerrno = ::listen(SockFd, SOMAXCONN);
  if(Returnerrno < 0)
  {
    LOG_OFF << "sockets::listen";
  }
}

inline int socketops::Accept(int Sockfd, struct sockaddr_in* addr)
{
  socklen_t addrLen = static_cast<socklen_t>(sizeof(*addr));
  int connfd = ::accept4(Sockfd, reinterpret_cast<struct sockaddr*>(addr), &addrLen, SOCK_NONBLOCK | SOCK_CLOEXEC);
  if(connfd < 0 && errno != EAGAIN)
  {
    int savedErrno = errno;
    if(savedErrno == EAGAIN)
    {
      return connfd;
    } 
    LOG_ERROR << "sockets::accept";
    switch(savedErrno)
    {
      case ECONNABORTED:
      case EINTR:
      case EPERM:
      case EMFILE:
        errno = savedErrno;
        break;
      case EBADF:
      case EFAULT:
      case EINVAL:
      case ENFILE:
      case ENOBUFS:
      case ENOMEM:
      case ENOTSOCK:
      case EOPNOTSUPP:
        LOG_FATAL << "unkownerrno sockets::accept" << savedErrno;
        break;
      default:
        LOG_FATAL << "unkownerrno sockets::accept" << savedErrno;    
    }
  } 
  return connfd;
}

inline ssize_t socketops::Read(int Sockfd, void* buf, size_t count)
{
  return ::read(Sockfd, buf, count);
}

inline ssize_t socketops::Readv(int Sockfd, const struct iovec* iov, int iovcnt)
{
  return ::readv(Sockfd, iov, iovcnt);
}

inline ssize_t socketops::Write(int Sockfd, const void* buf, size_t count)
{
  return ::write(Sockfd, buf, count);
}

inline ssize_t socketops::RecvFrom(int Sockfd, void* buf, size_t count, struct sockaddr_in* addr)
{
  return ::recvfrom(Sockfd, buf, count, 0, reinterpret_cast<sockaddr*>(addr), \
        reinterpret_cast<socklen_t*>(sizeof(struct sockaddr_in)));
}

inline ssize_t socketops::Sendto(int Sockfd, const void* buf, size_t count, struct sockaddr_in* addr)
{
  return ::sendto(Sockfd, buf, count, 0, reinterpret_cast<sockaddr*>(addr), sizeof(struct sockaddr_in));
}

inline ssize_t socketops::SendFile(int Outfd, int Intfd, off_t* offset, size_t count)
{
  return ::sendfile(Outfd, Intfd, offset, count);
}

inline void socketops::Close(int Sockfd)
{
  if(::close(Sockfd) < 0)
  {
    LOG_ERROR << "sockets::close";
  }
}

inline void socketops::ShutDownWrite(int Sockfd)
{
  if(::shutdown(Sockfd, SHUT_WR) < 0)
  {
    LOG_ERROR << "sockets::shutdwon";
  }
}

inline void socketops::SetKeepAlive(int Sockfd, bool ue)
{
  int op = ue ? 1 : 0;
  ::setsockopt(Sockfd, SOL_SOCKET, SO_KEEPALIVE, &op, static_cast<socklen_t>(sizeof(op)));
}

inline void socketops::SetTcpNoDelay(int Sockfd, bool ue)
{
  int op = ue ? 1 : 0;
  ::setsockopt(Sockfd, IPPROTO_TCP, TCP_NODELAY, &op, static_cast<socklen_t>(sizeof(op)));
}

inline void socketops::SetReuseAddr(int Sockfd, bool ue)
{
  int op = ue ? 1 : 0;
  ::setsockopt(Sockfd, SOL_SOCKET, SO_REUSEADDR, &op, static_cast<socklen_t>(sizeof(op)));
}

inline void socketops::SetReusePort(int Sockfd, bool ue)
{
  int op = ue ? 1 : 0;
  int ret = ::setsockopt(Sockfd, SOL_SOCKET, SO_REUSEPORT, &op, static_cast<socklen_t>(sizeof(op)));
  if(ret < 0 && ue)
  {
    LOG_ERROR << "sockets: setreuseport errno";
  }
}

inline int socketops::GetSocketError(int Sockfd)
{
  int optval;
  auto optlen = static_cast<socklen_t>(sizeof(optval));
  if(::getsockopt(Sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
  {
    return errno;
  }
  else
  {
    return optval;
  }
}

inline struct sockaddr_in GetLocalAddr(int Sockfd)
{
  struct sockaddr_in peeraddr;
  bzero(&peeraddr, sizeof(peeraddr));
  auto addrlen = static_cast<socklen_t>(sizeof(peeraddr));
  if(::getpeername(Sockfd, reinterpret_cast<struct sockaddr*>(&peeraddr), &addrlen) < 0)
  {
    LOG_ERROR << "sockets::GetLocalAddr";
  }
  return peeraddr;
}

inline struct sockaddr_in GetPeerAddr(int Sockfd)
{
  struct sockaddr_in localaddr;
  bzero(&localaddr, sizeof(localaddr));
  auto addrlen = static_cast<socklen_t>(sizeof(localaddr));
  if(::getsockname(Sockfd, reinterpret_cast<struct sockaddr*>(&localaddr), &addrlen) < 0)
  {
    LOG_ERROR << "sockets:GetPeerAddr";
  }
  return localaddr;
}