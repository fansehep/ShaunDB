#include "src/net/net_connection.hpp"
#include "src/net/net_server.hpp"

using ::fver::net::Server;
using ::fver::net::Connection;

class EchoServer {
public:
  EchoServer() = default;
  void Init(uint32_t port) {
    server_.setup(port);
  }
private:
  Server<Connection> server_;
};


int main () {


  return 0;
}