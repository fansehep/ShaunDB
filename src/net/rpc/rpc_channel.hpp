#ifndef SRC_NET_RPC_RPC_CHANNEL_H_
#define SRC_NET_RPC_RPC_CHANNEL_H_

#include <google/protobuf/service.h>
#include <google/protobuf/stubs/callback.h>

#include <map>
#include <mutex>

#include "src/net/connection.hpp"
#include "src/net/rpc/codeclite.hpp"
#include "src/net/rpc/rpc_code.hpp"

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
  RPCChannel();
  RPCChannel(net::ConnPtr);
  ~RPCChannel();
  void Init(
      net::ConnPtr& conn,
      const std::map<std::string, ::google::protobuf::Service*>* service_map);

  void CallMethod(const ::google::protobuf::MethodDescriptor* method_desptor,
                  ::google::protobuf::RpcController* rpc_conter,
                  const ::google::protobuf::Message* request,
                  ::google::protobuf::Message* response,
                  ::google::protobuf::Closure* done);

  int onMessage(net::ConnPtr conn);

  void setService(
      const std::map<std::string, ::google::protobuf::Service*>* sev) {
    services_ = sev;
  }

  void setConnection(net::ConnPtr conn) {
    conn_ = conn;
  }

 private:
  void onRPCMessage(net::ConnPtr conn,
                    const std::shared_ptr<frpc::RPCMessage>& message_ptr);

  void doneCallback(::google::protobuf::Message* response, int64_t id);

  struct OutStandingCall {
    ::google::protobuf::Message* response;
    ::google::protobuf::Closure* done;
  };

  RPCCodec codec_;
  net::ConnPtr conn_;
  std::atomic<int64_t> id_;

  std::mutex mtx_;
  std::map<int64_t, OutStandingCall> out_standings_;
  const std::map<std::string, ::google::protobuf::Service*>* services_;
};

using RPCChannelPtr = std::shared_ptr<RPCChannel>;

}  // namespace rpc

}  // namespace fver

#endif