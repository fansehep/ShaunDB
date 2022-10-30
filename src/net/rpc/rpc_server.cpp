#include "src/net/rpc/rpc_server.hpp"

#include <google/protobuf/descriptor.h>
#include <google/protobuf/service.h>

#include <functional>

#include "src/base/log/logging.hpp"
#include "src/net/connection.hpp"
#include "src/net/rpc/rpc_channel.hpp"

using ::std::placeholders::_1;

namespace fver {

namespace rpc {

void RPCServer::Init(uint32_t port) {
  tcp_server_.Init(port, nullptr, nullptr, nullptr, nullptr);
  tcp_server_.SetConnInitCallback(
      std::bind(&RPCServer::onConnection, this, _1));
}

void RPCServer::RegisterService(::google::protobuf::Service* service) {
  auto desc = service->GetDescriptor();
  rpc_service_[desc->full_name()] = service;
}

void RPCServer::Run() { tcp_server_.Run(); }

int RPCServer::onConnection(net::ConnPtr conn) {
  LOG_INFO("conn ip: {} ip: {} create connection", conn->getPeerIP(),
           conn->getPeerPort());
  RPCChannelPtr new_channel = std::make_shared<RPCChannel>(conn);
  new_channel->setService(&rpc_service_);
  tcp_server_.SetConnReadCallback(
      std::bind(&RPCChannel::onMessage, new_channel.get(), _1));
}

}  // namespace rpc

}  // namespace fver