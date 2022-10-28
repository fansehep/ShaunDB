#ifndef SRC_NET_ACTIVE_CONN_H_
#define SRC_NET_ACTIVE_CONN_H_

#include <functional>
#include <memory>
#include <string>

#include "src/base/noncopyable.hpp"
#include "src/net/conn_callback.hpp"
#include "src/net/connection.hpp"


using ::fver::base::NonCopyable;

namespace fver {

namespace net {

class NetServer;


using newConnectionCallback = std::function<void(int)>;

// 主动连接的封装
class Connectioner : public NonCopyable {
 public:
  Connectioner() = default;
  ~Connectioner();
  void Init(const std::string& ip, const uint32_t port,
            const std::shared_ptr<NetServer>& server, readHandle rh,
            writeHandle wh, closeHandle ch, timeoutHandle th);
  bool Run();
  void CloseConn();
  int getFd();
  const std::string& getPeerIP();
  uint32_t getPort();

 private:
  // 主动连接者共享 NetServer 的所有权
  std::shared_ptr<NetServer> server_;
  // 需要连接 IP
  std::string peer_ip_;
  // 需要连接的 port
  uint32_t peer_port_;
  // 包含一个 Connection;
  Connection conn_;
  //
  bool isRunning_;
};

}  // namespace net

}  // namespace fver

#endif