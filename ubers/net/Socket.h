#ifndef _UBERS_NET_SOCKET_
#define _UBERS_NET_SOCKET_
#include "Socket.h"
#include <boost/noncopyable.hpp>
#include "InetAddress.h"

namespace UBERS::net
{
class Socket : public boost::noncopyable
{
public:
  explicit Socket(int sockfd) : socketfd_(sockfd) { }

  ~Socket() { close(socketfd_); }

  int GetFd() const  {return this->socketfd_;}

  int Accept(InetAddress* addr);

  void BindAddress(const InetAddress& addr);

  void Listen();

  void ShutDownWrite();

  void SetTcpNagle(bool ue);

  void SetKeepAlive(bool ue);

  void SetReusePort(bool ue);

  void SetReuseAddr(bool ue);

  int GetSocketError();
private:
  const int socketfd_;
};

}

#endif