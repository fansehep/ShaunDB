#ifndef SRC_RAFT_RAFT_SERVER_H_
#define SRC_RAFT_RAFT_SERVER_H_

#include <vector>

#include "src/base/noncopyable.hpp"
#include "src/raft/raft_node.hpp"
using ::fver::base::NonCopyable;

namespace fver {

namespace raft {

struct RaftServerConfig;

class RaftServer : public NonCopyable {
 public:
  RaftServer() = default;
  void Init();


 private:
  // 当前 Raft 集群对应其他所有的服务器集群
  std::vector<RaftNode> vec_nodes_;
  // 当前 Raft 集群的数量
  int current_nodes_n_;
  // 自身的 角色
  RaftNode self_roles_;
};

}  // namespace raft

}  // namespace fver

#endif