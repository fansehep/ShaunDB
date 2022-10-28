#include "src/raft/raft_node.hpp"

#include "src/net/net_server.hpp"
#include "src/raft/raft.pb.h"

namespace fver {

namespace raft {

void RaftNode::Init(const std::string& ip, const uint32_t port,
                    const std::shared_ptr<NetServer>& server) {
  this->ip_ = ip;
  this->port_ = port;
}

}  // namespace raft

}  // namespace fver