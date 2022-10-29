#ifndef SRC_NET_RPC_RPC_CHANNEL_H_
#define SRC_NET_RPC_RPC_CHANNEL_H_

#include <google/protobuf/service.h>
#include <google/protobuf/stubs/callback.h>

#include <map>
#include <mutex>

#include "src/net/connection.hpp"

namespace google {

namespace protobuf {

class Descriptor;
class ServiceDescriptor;
class MethodDescriptor;
class Message;
class Closure;
class RpcController;
class Service;


}  // namespace protobuf

}  // namespace google

namespace fver {

namespace rpc {

class RPCChannel : public ::google::protobuf::RpcChannel {
 public:
  RPCChannel() = default;
  ~RPCChannel();
  void Init(
      net::Connection* conn,
      const std::map<std::string, ::google::protobuf::Service*>* service_map);

  void CallMethod(const ::google::protobuf::MethodDescriptor* method_desptor,
                  ::google::protobuf::RpcController* rpc_conter,
                  const ::google::protobuf::Message* request,
                  ::google::protobuf::Message* response,
                  ::google::protobuf::Closure* done);
  
  void onMessage();

 private:

  void onRPCMessage(net::Connection* conn, const std::shared_ptr<RPCMessage>& message_ptr);


  net::Connection* conn_;
};

}  // namespace rpc

}  // namespace fver

#endif