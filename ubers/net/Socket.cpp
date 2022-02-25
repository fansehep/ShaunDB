#include "Socket.h"
#include "SocketOps.h"

using namespace UBERS::net;


int Socket::Accept(InetAddress* addr)
{
  struct sockaddr_in6 addr6;
  auto conn =  sockets::Accept(socketfd_,&addr6);
  if(conn >= 0)
  {
    addr->SetSockAddrInet6(addr6);
  }
  return conn;
}

void Socket::BindAddress(const InetAddress& addr)
{
  sockets::BindOrDie(this->socketfd_, addr.GetSockAddr());
}


void Socket::Listen()
{
  sockets::ListenOrDie(this->socketfd_);
}

void Socket::ShutDownWrite()
{
  sockets::ShutDownWrite(this->socketfd_);
}

void Socket::SetTcpNagle(bool ue)
{
  sockets::SetTcpNoDelay(this->socketfd_, ue);
}

void Socket::SetKeepAlive(bool ue)
{
  sockets::SetKeepAlive(this->socketfd_, ue);
}

void Socket::SetReusePort(bool ue)
{
  sockets::SetReusePort(this->socketfd_, ue);
}

void Socket::SetReuseAddr(bool ue)
{
  sockets::SetReuseAddr(this->socketfd_, ue);
}

int Socket::GetSocketError()
{
  return sockets::GetSocketError(this->socketfd_);
}

