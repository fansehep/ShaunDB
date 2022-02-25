#include "../base/Logging.h"
#include "InetAddress.h"
#include "SocketOps.h"

#include <netdb.h>
#include <netinet/in.h>

constexpr static const in_addr_t kInaddrAny = INADDR_ANY;
constexpr static const in_addr_t kInaddrLoopback = INADDR_LOOPBACK;


using namespace UBERS;
using namespace UBERS::net;

InetAddress::InetAddress(uint16_t port, bool loopbackonly, bool ipv6)
{
  static_assert(offsetof(InetAddress, addr6_) == 0, "addr6_ offset 0");
  static_assert(offsetof(InetAddress, addr_) == 0, "addr_ offset 0");
  if(ipv6)
  {
    bzero(&addr6_, sizeof(addr6_));
    addr6_.sin6_family = AF_INET6;
    in6_addr ip = loopbackonly ? in6addr_loopback : in6addr_any;
    addr6_.sin6_addr = ip;
    addr6_.sin6_port = sockets::HostToNetwork16(port);
  }
  else
  {
    bzero(&addr_, sizeof(addr_));
    addr_.sin_family = AF_INET;
    in_addr_t ip = loopbackonly ? kInaddrLoopback : kInaddrAny;
    addr_.sin_addr.s_addr = sockets::HostToNetwork32(ip);
    addr_.sin_port = sockets::HostToNetwork16(port);
  }
}

InetAddress::InetAddress(std::string ip, uint16_t port, bool ipv6)
{
  if(ipv6 || strchr(ip.c_str(), ':'))
  {
    bzero(&addr6_, sizeof(addr6_));
    sockets::FromIpPort(ip.c_str(), port, &addr6_);
  }
  else
  {
    bzero(&addr_, sizeof(addr_));
    sockets::FromIpPort(ip.c_str(), port, &addr_);
  }
}

std::string InetAddress::toIpPort() const
{
  char buf[64] = "";
  sockets::ToIpPort(buf, sizeof(buf), GetSockAddr());
  return buf;
}

std::string InetAddress::toIp() const
{
  char buf[64] = "";
  sockets::ToIp(buf, sizeof(buf), GetSockAddr());
  return buf;
}

uint32_t InetAddress::IPV4_NetEndian() const
{
  assert(family() == AF_INET);
  return addr_.sin_addr.s_addr;
}

static thread_local char t_resolveBuffer[64 * 1024];

bool InetAddress::ReSolve(std::string hostname, InetAddress* result)
{
  assert(result != nullptr);
  struct hostent hent;
  struct hostent* he = nullptr;
  int herrno = 0;
  bzero(&hent, sizeof(hostent));
  
  int ret = gethostbyname_r(hostname.c_str(), &hent, t_resolveBuffer, sizeof(t_resolveBuffer), &he, &herrno);
  if(ret == 0 && he != nullptr)
  {
    assert(he->h_addrtype == AF_INET && he->h_length == sizeof(uint32_t));
    result->addr_.sin_addr = *reinterpret_cast<struct in_addr*>(he->h_addr);
    return true;
  }
  else
  {
    if(ret)
    {
      LOG_SYSERR << "InetAddress::ReSolve";
    }
    return false;
  }
}

void InetAddress::SetScopeId(uint32_t scope_id)
{
  if(family() == AF_INET6)
  {
    addr6_.sin6_scope_id = scope_id;
  }
}

