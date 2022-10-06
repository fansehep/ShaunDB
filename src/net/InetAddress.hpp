#ifndef SRC_BASE_INETADDRESS_H_
#define SRC_BASE_INETADDRESS_H_

#include <netinet/in.h>
#include <sys/socket.h>

#include <string>
#include <variant>

#include "src/base/NonCopyable.hpp"
namespace fver::net {

class InetAddress : public fver::base::NonCopyable {
 public:
  explicit InetAddress(uint16_t port = 0, bool ipv6 = false);
  InetAddress(const std::string& ip, uint16_t port);
  explicit InetAddress(const struct sockaddr_in& addr)
   : addr_(addr) {}
  explicit InetAddress(const struct sockaddr_in6& addr6)
   : addr6_(addr6) {}
  sa_family_t GetFamily();
  std::string ToIp();
  std::string ToIpPort();
  uint16_t GetPort();
 private:
  union {
    struct sockaddr_in addr_;
    struct sockaddr_in6 addr6_;
  };
};

}  // namespace fver::net

#endif
