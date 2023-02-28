#ifndef SRC_RAFT_RAFTCONF_H_
#define SRC_RAFT_RAFTCONF_H_
#include <string>
#include <unordered_map>
#include <vector>

namespace fver {

namespace raft {

struct PeerNode {
  std::string IP;
  uint32_t port;
};

struct RaftConfig {
  // 最小选举超时时间
  inline static uint64_t min_election_interval_ms = 700 * 1000;
  // 最大选举超时时间
  inline static uint64_t max_election_interval_ms = 1000 * 1000;
  // 心跳间隔时间
  inline static uint64_t append_interval_ms = 200 * 1000;
  // 当前集群的节点数量 3, 5, 7
  inline static uint64_t node_counts = 3;
  // RaftStorage 的配置, 具体为 rocksdb 的配置
  // rocksdb minor compaction 的线程数量
  inline static uint32_t storage_compaction_thread_counts = 2;
  // rocksdb log_dir
  inline static std::string storage_log_dir;
  // rocksdb db_dir
  inline static std::string storage_db_dir;
  // rocksdb wal_dir
  inline static std::string storage_wal_dir;
  
  
  RaftConfig() = default;

};

void ParseRaftConfig(const std::string& ip, const uint32_t port);

}  // namespace raft

}  // namespace fver

#endif