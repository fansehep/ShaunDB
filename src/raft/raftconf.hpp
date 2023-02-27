#ifndef SRC_RAFT_RAFTCONF_H_
#define SRC_RAFT_RAFTCONF_H_
#include <string>
#include <vector>

namespace fver {

namespace raft {

struct PeerNode {
  std::string IP;
  uint32_t port;
};

struct RaftConfig {
  // 当前节点的对外 IP
  std::string self_ip;
  // 当前节点的对外 Port
  uint32_t self_port;
  // 最小选举超时时间
  inline static uint64_t min_election_interval_ms = 700 * 1000;
  // 最大选举超时时间
  inline static uint64_t max_election_interval_ms = 1000 * 1000;
  // 心跳间隔时间
  inline static uint64_t append_interval_ms = 200 * 1000;
  // 当前集群的节点数量 3, 5, 7
  inline static uint64_t node_counts = 3;
  RaftConfig() = default;
};

void ParseRaftConfig(struct RaftConfig* conf);

}  // namespace raft

}  // namespace fver

#endif