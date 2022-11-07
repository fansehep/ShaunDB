#ifndef SRC_RAFT_RAFT_NODE_H_
#define SRC_RAFT_RAFT_NODE_H_

#include <string>

#include "src/base/noncopyable.hpp"
#include "src/base/timestamp.hpp"
#include "src/net/active_conn.hpp"
#include "src/net/net_server.hpp"
#include "src/net/repeated_timer.hpp"
#include "src/raft/raft.pb.h"
#include <thread>

using ::fver::base::NonCopyable;
using ::fver::base::TimeStamp;
using ::fver::net::Connectioner;
using ::fver::net::NetServer;
using ::fver::net::RepeatedTimer;

namespace fver {

namespace raft {

enum RaftRoles {
  // 领导者
  kLeader = 0,
  // 追随者
  kFollower = 1,
  // 候选者
  kCandidates = 2,
};

const std::string kRaftRolesStr[] = {
    "Leader",
    "Follower",
    "Candidates",
};

class RaftServer;

// 单个 RaftNode 通过Connectioner持有 NetServer 的所有权
class RaftNode : public NonCopyable {
 public:
  RaftNode() = default;
  ~RaftNode() = default;
  void Init(const std::string& ip, const uint32_t port,
            const std::shared_ptr<NetServer>& server);
  // 发起选举
  void RequetedVote(const RaftMes::RequestVoteArgs& request_vote_args);

  // 追加日志 or 心跳
  void AppendEntries(const RaftMes::AppendEntriesArgs& append_log_args);

  // 发送快照
  void SendSnapshot(const RaftMes::InstallSnapshotArgs& install_snapshot_args);

  RaftRoles getRoles();

  std::string getRolesStr();

  uint64_t getTerm();

 private:

  // TODO: 背景线程, 尽可能地不阻塞主要服务
  std::thread bg_thread_;

  // 连接者
  Connectioner conner_;

  // 对等方的 端口
  uint32_t port_;

  // 远程节点 所对应的 hostname
  std::string ip_;

  // 远程节点 所对应的 角色
  RaftRoles self_roles_;

  // 远程节点 所对应的 term
  uint64_t term_;


  // 最后活跃时间戳
  TimeStamp last_timestamp_;



  // 当前投票给了哪个节点
  std::pair<std::string, uint32_t> current_vote_for_;

  // for thread_safe
  std::mutex mtx_;

};

}  // namespace raft

}  // namespace fver

#endif