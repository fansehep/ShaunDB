#include <sys/uio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <netinet/tcp.h>
#include <sys/sendfile.h>
#include "SocketOps.h"
#include "../base/Logging.h"

using namespace UBERS::net;


inline int sockets::CreateSocket(sa_family_t faml)
{
  if(faml == AF_INET)
  {
    return CreateStreamNonBlockOrDie();
  }
  else
  {
    return CreateDgramNonBlockingOrDie();
  }
}

inline int sockets::CreateStreamNonBlockOrDie()
{
  int Sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
  if(Sockfd < 0)
  {
    LOG_SYSERR << "sockets::CreateStreamNonBlockOrDie";
  }
  return Sockfd;
}

inline int sockets::CreateDgramNonBlockingOrDie()
{
  int Sockfd = ::socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_UDP);
  if(Sockfd < 0)
  {
    LOG_SYSERR << "sockets::CreateDgramNonBlockingOrDie";
  }
  return Sockfd;
}

inline int sockets::Connect(int ClientSockfd, struct sockaddr* addr)
{
  return ::connect(ClientSockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in)));
}

inline void sockets::BindOrDie(int ClientSockfd, struct sockaddr* addr)
{
  int Returnerrno = ::bind(ClientSockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in)));
  if(Returnerrno < 0)
  {
    LOG_SYSERR << "sockets::Bind";
  }
}

inline void sockets::ListenOrDie(int SockFd)
{
  int Returnerrno = ::listen(SockFd, SOMAXCONN);
  if(Returnerrno < 0)
  {
    LOG_SYSERR << "sockets::listen";
  }
}

inline int sockets::Accept(int Sockfd, struct sockaddr_in6* addr)
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

inline ssize_t sockets::Read(int Sockfd, void* buf, size_t count)
{
  return ::read(Sockfd, buf, count);
}

inline ssize_t sockets::Readv(int Sockfd, const struct iovec* iov, int iovcnt)
{
  return ::readv(Sockfd, iov, iovcnt);
}

inline ssize_t sockets::Write(int Sockfd, const void* buf, size_t count)
{
  return ::write(Sockfd, buf, count);
}

inline ssize_t sockets::RecvFrom(int Sockfd, void* buf, size_t count, struct sockaddr_in* addr)
{
  return ::recvfrom(Sockfd, buf, count, 0, reinterpret_cast<sockaddr*>(addr), \
        reinterpret_cast<socklen_t*>(sizeof(struct sockaddr_in)));
}

inline ssize_t sockets::Sendto(int Sockfd, const void* buf, size_t count, struct sockaddr_in* addr)
{
  return ::sendto(Sockfd, buf, count, 0, reinterpret_cast<sockaddr*>(addr), sizeof(struct sockaddr_in));
}

inline ssize_t sockets::SendFile(int Outfd, int Intfd, off_t* offset, size_t count)
{
  return ::sendfile(Outfd, Intfd, offset, count);
}

inline void sockets::Close(int Sockfd)
{
  if(::close(Sockfd) < 0)
  {
    LOG_ERROR << "sockets::close";
  }
}

inline void sockets::ShutDownWrite(int Sockfd)
{
  if(::shutdown(Sockfd, SHUT_WR) < 0)
  {
    LOG_ERROR << "sockets::shutdwon";
  }
}

inline void sockets::SetKeepAlive(int Sockfd, bool ue)
{
  int op = ue ? 1 : 0;
  ::setsockopt(Sockfd, SOL_SOCKET, SO_KEEPALIVE, &op, static_cast<socklen_t>(sizeof(op)));
}

inline void sockets::SetTcpNoDelay(int Sockfd, bool ue)
{
  int op = ue ? 1 : 0;
  ::setsockopt(Sockfd, IPPROTO_TCP, TCP_NODELAY, &op, static_cast<socklen_t>(sizeof(op)));
}

inline void sockets::SetReuseAddr(int Sockfd, bool ue)
{
  int op = ue ? 1 : 0;
  ::setsockopt(Sockfd, SOL_SOCKET, SO_REUSEADDR, &op, static_cast<socklen_t>(sizeof(op)));
}

inline void sockets::SetReusePort(int Sockfd, bool ue)
{
  int op = ue ? 1 : 0;
  int ret = ::setsockopt(Sockfd, SOL_SOCKET, SO_REUSEPORT, &op, static_cast<socklen_t>(sizeof(op)));
  if(ret < 0 && ue)
  {
    LOG_ERROR << "sockets: setreuseport errno";
  }
}

inline int sockets::GetSocketError(int Sockfd)
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

inline struct sockaddr_in sockets::GetLocalAddr(int Sockfd)
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

inline struct sockaddr_in sockets::GetPeerAddr(int Sockfd)
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

void sockets::ToIpPort(char* buf, size_t size, const struct sockaddr* addr)
{
  if(addr->sa_family == AF_INET6)
  {
    buf[0] = '[';
    ToIp(buf + 1, size - 1, addr);
    size_t end = strlen(buf);
    const struct sockaddr_in6* addr6 = (const struct sockaddr_in6*)(addr);
    uint16_t port = sockets::NetworkToHost16(addr6->sin6_port);
    assert(size > end);
    snprintf(buf + end, size - end, "]:%u", port);
    return ;
  }
  ToIp(buf, size, addr);
  size_t end = ::strlen(buf);
  const struct sockaddr_in* addr4 = (const struct sockaddr_in*)(addr);
  uint16_t port = sockets::NetworkToHost16(addr4->sin_port);
  snprintf(buf + end, size - end, ":%u", port);
}

void sockets::ToIp(char* buf, size_t size, const struct sockaddr* addr)
{
  if(addr->sa_family == AF_INET)
  {
    assert(size >= INET_ADDRSTRLEN);
    const struct sockaddr_in* addr4 = (const struct sockaddr_in*)(addr);
    ::inet_ntop(AF_INET, &addr4->sin_addr, buf, static_cast<socklen_t>(size));
  }
  else if(addr->sa_family == AF_INET6)
  {
    assert(size >= INET6_ADDRSTRLEN);
    const struct sockaddr_in6* addr6 = (const struct sockaddr_in6*)(addr);
    ::inet_ntop(AF_INET6, &addr6->sin6_addr, buf, static_cast<socklen_t>(size));
  }
}
void sockets::FromIpPort(const char* ip, uint16_t port, struct sockaddr_in* addr)
{
  addr->sin_family = AF_INET;
  addr->sin_port = HostToNetwork16(port);
  if(::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0)
  {
    LOG_SYSERR << "sockets::FromIpPort";
  }
}
void sockets::FromIpPort(const char* ip, uint16_t port, struct sockaddr_in6* addr)
{
   addr->sin6_family = AF_INET6;
   addr->sin6_port = HostToNetwork16(port);
   if(::inet_pton(AF_INET6, ip, &addr->sin6_addr) <= 0)
   {
     LOG_SYSERR <<"sockets::FromIpPort";
   }
}


