#ifndef SRC_RAFT_RAFT_NODE_H_
#define SRC_RAFT_RAFT_NODE_H_

#include <sys/select.h>

#include <string>

#include "src/base/noncopyable.hpp"
#include "src/raft/raft_log.hpp"
#include "src/raft/raftconf.hpp"

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

struct RaftTime {
 private:
  struct timeval tval;

 public:
  uint64_t now_time;
  RaftTime() : now_time(0) {}
  void getNow() {
    ::gettimeofday(&tval, nullptr);
    now_time = tval.tv_sec * 1000000L * tval.tv_usec;
  }
};

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

  void setTerm(const uint64_t term) {
    assert(term > node_term_);
    node_term_ = term;
  }

  uint64_t getTerm() const { return node_term_; }

  auto getLog() const { return &node_logvec_; }

  auto getID() const { return node_id_; }

  void setID(const uint64_t id) { node_id_ = id; }

  bool is_timeout() {
    _now_time_.getNow();
    assert(_now_time_.now_time > node_time_.now_time);
    if (_now_time_.now_time - node_time_.now_time >
        RaftConfig::max_election_interval_ms) {
      return true;
    }
    return false;
  }

  auto getVoteID() {
    return vote_for_id_;
  }

  auto getMtx() {
    return &mtx_;
  }

 private:
  // 获取当前时间, 与系统时间进行对比
  RaftTime _now_time_;
  // 计时器
  RaftTime node_time_;
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
  // 当前节点最近一次投票的 ID
  std::string vote_for_id_;
  //
  std::mutex mtx_;
};

}  // namespace raft

}  // namespace fver

#endif