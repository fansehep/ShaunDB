#include "src/net/rpc/codeclite.hpp"

#include <event2/util.h>
#include <zlib.h>

#include "src/net/net_server.hpp"
#include "src/net/rpc/google-inl.hpp"

namespace fver {

namespace rpc {

void ProtobufCodeLite::Send(net::ConnPtr conn,
                            const ::google::protobuf::Message &message) {
  auto e_buf = evbuffer_new();
  if (fillEmptyBuffer(e_buf, message) < 0) {
    LOG_ERROR("send fill empty error {}",
              evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
  }
  if (conn)
}

}  // namespace rpc

}  // namespace fver