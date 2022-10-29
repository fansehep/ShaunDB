#ifndef SRC_NET_RPC_RPC_SERVER_H_
#define SRC_NET_RPC_RPC_SERVER_H_

extern "C" {
#include <event2/rpc.h>
#include <event2/rpc_compat.h>
#include <event2/rpc_struct.h>
}

#include "src/base/noncopyable.hpp"
#include "src/net/net_server.hpp"

using ::fver::base::NonCopyable;

namespace google {

namespace protobuf {

class Service;

}

}  // namespace google

namespace fver {

namespace rpc {

class RPCServer : public NonCopyable {
 public:
  RPCServer() = default;
  void Init(uint32_t port);

  void RegisterService(::google::protobuf::Service*);

 private:
  net::NetServer tcp_server_;
  std::map<std::string, ::google::protobuf::Service*> rpc_service_;
};

}  // namespace rpc

}  // namespace fver

#endif
