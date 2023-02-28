#ifndef SRC_RAFT_RAFT_SERVER_H_
#define SRC_RAFT_RAFT_SERVER_H_

#include <vector>

#include "src/base/noncopyable.hpp"
#include "src/net/net_server.hpp"
#include "src/net/repeated_timer.hpp"
#include "src/raft/raft_node.hpp"
#include "src/raft/raft_service.hpp"

using ::fver::base::NonCopyable;
using ::fver::net::RepeatedTimer;

namespace fver {

namespace raft {

struct RaftConfig;
class RaftPeerNode;

class RaftServer final : public NonCopyable {
 public:
  /**
   * @net_server: need net_server(epoll) trigger timer
   * @config: read config
   */
  bool Init(const std::shared_ptr<net::NetServer>& net_server,
            const RaftConfig& config);

 private:
  /**
   * @brief: use repeated timer trigger on time
   */
  int _RequestVoteOnTimed(RepeatedTimer* repeated_timer);

  int _AppendEntriesOnTimed(RepeatedTimer* repeated_timer);

  // timer trigger for request vote
  RepeatedTimer RequestVoteTimer_;
  // timer trigger for appendtries
  RepeatedTimer AppendEntriesTimer_;
  // peer server vec
  std::vector<RaftPeerNode> peerNodeVec_;
  RaftServiceImpl raft_service_impl_;
  RaftNode raft_node_;
};

}  // namespace raft

}  // namespace fver

#endif