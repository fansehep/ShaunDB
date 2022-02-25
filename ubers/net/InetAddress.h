#ifndef _UBERS_NET_INETADDRESS_H_
#define _UBERS_NET_INETADDRESS_H_
#include <netinet/in.h>
#include <boost/noncopyable.hpp>
#include <string>
namespace UBERS::net
{

class InetAddress
{
public:
  explicit InetAddress(uint16_t port = 0, bool loopbackonly = false, bool ipv6 = false);
  explicit InetAddress(const struct sockaddr_in6& addr) : addr6_(addr) { }
  explicit InetAddress(const struct sockaddr_in& addr) : addr_(addr) { }
  InetAddress(std::string ip, uint16_t port, bool ipv6 = false);

  sa_family_t family() const { return addr_.sin_family; }
  std::string toIp() const;
  std::string toIpPort() const;
  uint16_t Port() const;

  struct sockaddr* GetSockAddr() const { return (sockaddr*)(&addr6_); }
  void SetSockAddrInet6(const struct sockaddr_in6& addr6) { this->addr6_ = addr6;}

  uint32_t IPV4_NetEndian() const;
  uint16_t Port_NetEndian() const { return addr_.sin_port;}

  static bool ReSolve(std::string hostname, InetAddress* result);

  void SetScopeId(uint32_t scope_id);
private:
  union 
  {
    struct sockaddr_in addr_;
    struct sockaddr_in6 addr6_;
  };
};
}
#endif