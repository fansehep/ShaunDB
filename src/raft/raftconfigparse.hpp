#ifndef SRC_RAFT_RAFTCONFIGPARSE_H_
#define SRC_RAFT_RAFTCONFIGPARSE_H_
#include <string>
#include <vector>

namespace fver {

namespace raft {

struct PeerNode {
  std::string IP;
  uint32_t port;
};

struct RaftServerConfig {
  // 自身的配置
  PeerNode current_node_;
  // 集群中的其他节点的集合
  std::vector<PeerNode> vec_nodes;
  // 最小选举超时时间
  uint64_t min_election_interval_ms;
  // 最大选举超时时间
  uint64_t max_election_interval_ms;
  // 心跳间隔时间
  uint64_t append_interval_ms;
  // 默认配置
  RaftServerConfig()
      : min_election_interval_ms(700),
        max_election_interval_ms(1000),
        append_interval_ms(200) {}
};

void ParseRaftServerConfig(struct RaftServerConfig* conf);

}  // namespace raft

}  // namespace fver

#endif