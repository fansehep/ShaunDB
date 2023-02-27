
#include <grpcpp/client_context.h>

#include "grpcpp/grpcpp.h"
#include "src/base/log/logging.hpp"
#include "src/raft/test/testkv.grpc.pb.h"

class TestTbkvClient {
 public:
  TestTbkvClient(const std::shared_ptr<grpc::Channel>& channel)
      : server_stub_(tbkv::TbkvService::NewStub(channel)) {}

  bool SetKv(const std::string& k, const std::string& v) {
    tbkv::SetKeyRequest req;
    req.set_key(k);
    req.set_value(v);
    tbkv::SetKeyReply rep;
    grpc::ClientContext cont;
    auto status = server_stub_->SetKv(&cont, req, &rep);
    if (true == status.ok()) {
      return true;
    }
    return false;
  }

  bool GetKv(const std::string& k, std::string* v) {
    tbkv::GetKeyRequest req;
    req.set_key(k);
    grpc::ClientContext cont;
    tbkv::GetKeyReply rep;
    auto status = server_stub_->GetKv(&cont, req, &rep);
    if (true == status.ok()) {
      if (-1 == rep.status()) {
        return false;
      }
      *v = rep.value();
      return true;
    }
    return false;
  }

 private:
  std::string server_info;
  std::unique_ptr<tbkv::TbkvService::Stub> server_stub_;
};

int main() {
  std::string server_host = "127.0.0.1:88880";
  TestTbkvClient client(
      grpc::CreateChannel(server_host, grpc::InsecureChannelCredentials()));
  auto ue = client.SetKv("123", "456");
  if (false == ue) {
    LOG_ERROR("client set error");
  }
  std::string res;
  client.GetKv("123", &res);
  LOG_INFO("client get key: 123, value: {}", res);
  return 0;
}