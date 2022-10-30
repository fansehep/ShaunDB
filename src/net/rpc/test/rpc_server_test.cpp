#include "src/net/rpc/rpc_server.hpp"

#include <google/protobuf/service.h>
#include <google/protobuf/stubs/callback.h>

#include <thread>

#include "src/base/log/logging.hpp"
#include "src/net/rpc/test/set_kv.pb.h"

namespace fver {

namespace rpc {

class TestKVSerivce : public kvtest::kvService {
 public:
  virtual void Sut(::google::protobuf::RpcController* con_troller,
                   const kvtest::SetRequest* set_req,
                   kvtest::SetReply* set_reply,
                   ::google::protobuf::Closure* done) {
    LOG_DEBUG("request debug string: {}", set_req->DebugString());
    LOG_TRACE("set request key: {} value: {}", set_req->key(),
              set_req->value());
    storage_.insert({set_req->key(), set_req->value()});
    done->Run();
  }

  virtual void Get(::google::protobuf::RpcController* con_troller,
                   const kvtest::GetRequest* get_req,
                   kvtest::GetReply* get_reply,
                   ::google::protobuf::Closure* done) {
    LOG_TRACE("get reqeust key: {} ", get_req->key());
    auto iter = storage_.find(get_req->key());
    if (iter == storage_.end()) {
      get_reply->set_status("Not found!");

    } else {
      get_reply->set_value(iter->second);
    }
    done->Run();
  }

  std::map<std::string, std::string> storage_;
};

}  // namespace rpc

}  // namespace fver

int main() {
  fver::rpc::TestKVSerivce service;
  fver::rpc::RPCServer rpc_server_;
  rpc_server_.Init(9090);
  rpc_server_.RegisterService(&service);
  rpc_server_.Run();
  std::this_thread::sleep_for(std::chrono::seconds(60));
}