#include <thread>
#include "src/base/log/logging.hpp"
#include "src/base/noncopyable.hpp"
#include "src/net/active_conn.hpp"
#include "src/net/connection.hpp"
#include "src/net/net_server.hpp"
#include "src/net/rpc/rpc_channel.hpp"
#include "src/net/rpc/test/set_kv.pb.h"

using ::fver::base::NonCopyable;
using ::std::placeholders::_1;
using ::std::placeholders::_2;

namespace fver {

namespace rpc {

class RPCClient : public NonCopyable {
 public:
  RPCClient(const std::string& hostname, const uint32_t port)
      : channel_ptr_(std::make_shared<RPCChannel>()),
        stub_(channel_ptr_.get()) {
    server_ = std::make_shared<net::NetServer>();
    server_->Init(port + 10000, nullptr, nullptr, nullptr,
                  std::bind(&RPCChannel::onMessage, channel_ptr_.get(), _1));
    active_conner.Init(
        hostname, port, server_,
        std::bind(&RPCChannel::onMessage, channel_ptr_.get(), _1), nullptr,
        nullptr, nullptr);
    server_->SetConnInitCallback(std::bind(&RPCClient::onConnection, this, _1));
  }

  void Run() {
    bg_thread_ = std::thread([&]() { server_->Run(); });
    active_conner.Run();
  }

 private:
  std::thread bg_thread_;
  int onConnection(net::ConnPtr conn) {
    channel_ptr_->setConnection(conn);
    set_req.set_key("123");
    set_req.set_value("123");
    stub_.Set(
        nullptr, &set_req, &set_reply,
        google::protobuf::NewCallback(this, &RPCClient::solved, &set_reply));
  }
  void solved(kvtest::SetReply* resp) {
    LOG_INFO("soloved :", resp->DebugString());
  }

  kvtest::SetReply set_reply;

  kvtest::SetRequest set_req;

  std::shared_ptr<net::NetServer> server_;
  net::Connectioner active_conner;
  RPCChannelPtr channel_ptr_;
  kvtest::kvService_Stub stub_;
};

}  // namespace rpc

}  // namespace fver

int main () {
  fver::rpc::RPCClient client_("127.0.0.1", 9090);
  client_.Run();
  std::this_thread::sleep_for(std::chrono::seconds(10));
  return 0;
}