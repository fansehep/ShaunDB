#include <grpcpp/server_context.h>

#include <unordered_map>

#include "grpcpp/grpcpp.h"
#include "src/base/log/logging.hpp"
#include "src/raft/test/testkv.grpc.pb.h"

class TbkvServiceImp final : public tbkv::TbkvService::Service {
 public:
  grpc::Status SetKv(grpc::ServerContext* cont, const tbkv::SetKeyRequest* req,
                     tbkv::SetKeyReply* reply) override {
    kv_map_.insert({req->key(), req->value()});
    reply->set_status(1);
    return grpc::Status::OK;
  }

  grpc::Status GetKv(grpc::ServerContext* cont, const tbkv::GetKeyRequest* req,
                     tbkv::GetKeyReply* reply) override {
    auto find_iter = kv_map_.find(req->key());
    if (kv_map_.end() == find_iter) {
      reply->set_status(-1);
      return grpc::Status::OK;
    }
    reply->set_value(find_iter->second);
    return grpc::Status::OK;
  }

 private:
  std::unordered_map<std::string, std::string> kv_map_;
};

int main() {
  std::string server_info = "127.0.0.1:88880";
  TbkvServiceImp service_imp;
  grpc::EnableDefaultHealthCheckService(true);
  grpc::ServerBuilder builder;
  builder.AddListeningPort(server_info, grpc::InsecureServerCredentials());
  builder.RegisterService(&service_imp);
  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  LOG_INFO("server listening on {}", server_info);
  server->Wait();
  return 0;
}