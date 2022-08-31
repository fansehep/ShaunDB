#ifndef SRC_NET_SOCKET_H_
#define SRC_NET_SOCKET_H_

#include "src/base/NonCopyable.hpp"

namespace fver::net {

class Socket : public fver::base::NonCopyable {
 public:
  
 private:
  int fd;
};

}  // namespace fver::net

#endif