#ifndef SRC_NET_RPC_RPC_CODE_H_
#define SRC_NET_RPC_RPC_CODE_H_

#include "src/net/rpc/rpc.pb.h"
namespace fver {

namespace rpc {


class RPCMessage;
extern const char rpc_tage[];


using RPCCodec = ProtobufCodecLiteT<RPCMessage, rpc_tage>;

}



}

#endif