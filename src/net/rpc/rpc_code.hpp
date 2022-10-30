#ifndef SRC_NET_RPC_RPC_CODE_H_
#define SRC_NET_RPC_RPC_CODE_H_

#include "src/net/rpc/codeclite.hpp"
#include "src/net/rpc/rpc.pb.h"


namespace fver {

namespace rpc {

static constexpr char rpc_tag[] = "RPC0";

using RPCCodec = ProtobufCodeLiteT<frpc::RPCMessage, rpc_tag>;

}  // namespace rpc

}  // namespace fver

#endif