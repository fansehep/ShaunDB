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

  
 private:
  union {
    struct sockaddr_in addr_;
    struct sockaddr_in6 addr6_;
  };
};

}  // namespace fver::net

#endif