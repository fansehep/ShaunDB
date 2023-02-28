#ifndef SRC_RAFT_RAFT_PEER_H_
#define SRC_RAFT_RAFT_PEER_H_

#include <grpcpp/grpcpp.h>

#include <boost/fiber/future/promise.hpp>
#include <future>

#include "src/raft/fiber_define.hpp"
#include "src/raft/raft.grpc.pb.h"
#include "src/raft/raft_node.hpp"

namespace fver {

namespace raft {

class RaftNode;
struct RaftLogInfo;

class RaftPeerNode {
 public:
  explicit RaftPeerNode(const std::shared_ptr<grpc::Channel>& channel)
      : peer_stub_(RaftMes::RaftService::NewStub(channel)) {}

  /**
   * @brief: init a peer raftserver in local
   *         fail return false.
   * @peer_ip: peer node server ip
   * @peer_port: peer node server port
   */
  bool Init(const std::string& peer_ip, const uint32_t peer_port);

  void RequestVote(
      const RaftNode& node,
      const std::shared_ptr<boost::fibers::promise<RaftMes::RequestVoteReply>>&
          reply) const;

  void AppendEntries(const RaftNode& node);

  void InstallSnapshot(const RaftNode& node);

 private:
  // peer server info like ip:port
  std::string server_info_;
  // grpc stub
  std::unique_ptr<RaftMes::RaftService::Stub> peer_stub_;
};

}  // namespace raft

}  // namespace fver

#endif