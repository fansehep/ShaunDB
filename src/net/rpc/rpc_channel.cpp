#include "src/net/rpc/rpc_channel.hpp"

#include <google/protobuf/descriptor.h>

#include "src/net/connection.hpp"
#include "src/net/rpc/rpc.pb.h"

namespace fver {

namespace rpc {

RPCChannel::RPCChannel()
    : codec_(std::bind(&RPCChannel::onRPCMessage, this, std::placeholders::_1,
                       std::placeholders::_2)),
      services_(nullptr) {}

RPCChannel::RPCChannel(net::ConnPtr conn)
    : codec_(std::bind(&RPCChannel::onRPCMessage, this, std::placeholders::_1,
                       std::placeholders::_2)),
      conn_(conn),
      services_(NULL) {}

RPCChannel::~RPCChannel() {
  for (std::map<int64_t, OutStandingCall>::iterator it = out_standings_.begin();
       it != out_standings_.end(); ++it) {
    OutStandingCall out = it->second;
    delete out.response;
    delete out.done;
  }
}

// Call the given method of the remote service.  The signature of this
// procedure looks the same as Service::CallMethod(), but the requirements
// are less strict in one important way:  the request and response objects
// need not be of any specific class as long as their descriptors are
// method->input_type() and method->output_type().
void RPCChannel::CallMethod(const ::google::protobuf::MethodDescriptor* method,
                            google::protobuf::RpcController* controller,
                            const ::google::protobuf::Message* request,
                            ::google::protobuf::Message* response,
                            ::google::protobuf::Closure* done) {
  frpc::RPCMessage message;
  message.set_type(frpc::kRequest);
  int64_t id = id_++;
  message.set_id(id);
  message.set_service(method->service()->full_name());
  message.set_method(method->name());
  message.set_request(request->SerializeAsString());  // FIXME: error check

  OutStandingCall out = {response, done};
  {
    std::lock_guard<std::mutex> lk(mtx_);
    out_standings_[id] = out;
  }
  codec_.Send(conn_, message);
}

int RPCChannel::onMessage(net::ConnPtr conn) {
  codec_.onMessage(conn);
  return 1;
}

void RPCChannel::onRPCMessage(
    net::ConnPtr conn, const std::shared_ptr<frpc::RPCMessage>& messagePtr) {
  assert(conn == conn_);
  // printf("%s\n", message.DebugString().c_str());
  frpc::RPCMessage& message = *messagePtr;
  if (message.type() == frpc::kResponse) {
    int64_t id = message.id();
    assert(message.has_response() || message.has_error_code());

    OutStandingCall out = {NULL, NULL};

    {
      std::lock_guard<std::mutex> lk(mtx_);
      std::map<int64_t, OutStandingCall>::iterator it = out_standings_.find(id);
      if (it != out_standings_.end()) {
        out = it->second;
        out_standings_.erase(it);
      }
    }

    if (out.response) {
      std::unique_ptr<google::protobuf::Message> d(out.response);
      if (message.has_response()) {
        out.response->ParseFromString(message.response());
      }
      if (out.done) {
        out.done->Run();
      }
    }
  } else if (message.type() == frpc::kRequest) {
    // FIXME: extract to a function
    frpc::ErrorCode error = frpc::kParseError;
    if (services_) {
      std::map<std::string, google::protobuf::Service*>::const_iterator it =
          services_->find(message.service());
      if (it != services_->end()) {
        google::protobuf::Service* service = it->second;
        assert(service != NULL);
        const google::protobuf::ServiceDescriptor* desc =
            service->GetDescriptor();
        const google::protobuf::MethodDescriptor* method =
            desc->FindMethodByName(message.method());
        if (method) {
          std::unique_ptr<google::protobuf::Message> request(
              service->GetRequestPrototype(method).New());
          if (request->ParseFromString(message.request())) {
            google::protobuf::Message* response =
                service->GetResponsePrototype(method).New();
            // response is deleted in doneCallback
            int64_t id = message.id();
            service->CallMethod(
                method, NULL, request.get(), response,
                NewCallback(this, &RPCChannel::doneCallback, response, id));
            error = frpc::kOk;
          } else {
            error = frpc::kInvalid_request;
          }
        } else {
          error = frpc::kNoMethod;
        }
      } else {
        error = frpc::kNoMethod;
      }
    } else {
      error = frpc::kNoService;
    }
    if (error != frpc::kOk) {
      frpc::RPCMessage response;
      response.set_type(frpc::kResponse);
      response.set_id(message.id());
      response.set_error_code(error);
      codec_.Send(conn_, response);
    }
  } else if (message.type() == frpc::kError) {
  }
}

void RPCChannel::doneCallback(::google::protobuf::Message* response,
                              int64_t id) {
  std::unique_ptr<google::protobuf::Message> d(response);
  frpc::RPCMessage message;
  message.set_type(frpc::kResponse);
  message.set_id(id);
  message.set_response(response->SerializeAsString());  // FIXME: error check
  codec_.Send(conn_, message);
}

}  // namespace rpc
}  // namespace fver