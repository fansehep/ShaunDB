#include "src/net/rpc/rpc_server.hpp"

#include <google/protobuf/descriptor.h>
#include <google/protobuf/service.h>


namespace fver {


namespace rpc {



void RPCServer::RegisterService(::google::protobuf::Service * service) {
  auto desc = service->GetDescriptor();
  rpc_service_[desc->full_name()] = service;
}








}




}