#ifndef SRC_RAFT_RAFT_NODE_H_
#define SRC_RAFT_RAFT_NODE_H_

#include <string>

#include "src/base/noncopyable.hpp"
#include "src/raft/raft_log.hpp"

extern "C" {
#include <assert.h>
}

using ::fver::base::NonCopyable;

namespace fver {

namespace raft {

enum RaftRoles {
  // 领导者
  kLeader = 0,
  // 追随者
  kFollower = 1,
  // 候选者
  kCandidates = 2,
  // 学习者
  kLearner = 3,
};

const std::string kRaftRolesStr[] = {
    "Leader",
    "Follower",
    "Candidates",
    "Learner",
};

class RaftServer;

class RaftNode : public NonCopyable {
 public:
  RaftNode() = default;
  ~RaftNode() = default;

  RaftRoles getRoles() const { return raft_role_; }

  void be_Leader() { raft_role_ = RaftRoles::kLeader; }

  void be_Follower() { raft_role_ = RaftRoles::kFollower; }

  void be_Candidates() { raft_role_ = RaftRoles::kCandidates; }

  void be_Learner() { raft_role_ = RaftRoles::kLearner; }

  RaftRoles get_Roles() { return raft_role_; }

  /**
   * @brief: for log
   */
  const std::string_view get_RolesStr() {
    assert(raft_role_ >= 0 && raft_role_ <= 3);
    return kRaftRolesStr[raft_role_];
  }

  std::string_view get_NodeIp() const { return node_ip_; }

  uint32_t get_NodePort() const { return node_port_; }
  /**
   * @brief: init
   */
  bool Init(const std::string& node_ip, const uint32_t node_port);

  void addTerm() { node_term_++; }

  uint64_t getTerm() const { return node_term_; }

  auto getLog() const { return &node_logvec_; }

  auto getID() const {
    return node_id_;
  }

  void setID(const uint64_t id) {
    node_id_ = id;
  }

 private:
  // 当前节点的 ID
  uint64_t node_id_;
  // 当前节点的角色
  RaftRoles raft_role_;
  // 当前的 IP
  std::string node_ip_;
  // 当前监听的端口
  uint32_t node_port_;
  // 当前节点的term
  uint64_t node_term_;
  // 当前节点的日志
  RaftLog node_logvec_;
};

}  // namespace raft

}  // namespace fver

#endif