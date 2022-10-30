#include "src/net/rpc/rpc_code.hpp"

#include "src/net/rpc/google-inl.hpp"

namespace {
int ProtobufVersionCheck() {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  return 0;
}
int dummy __attribute__((unused)) = ProtobufVersionCheck();
}  // namespace


