#ifndef SRC_RAFT_RAFT_SERVER_H_
#define SRC_RAFT_RAFT_SERVER_H_

#include <vector>

#include "src/base/noncopyable.hpp"
#include "src/raft/raft_node.hpp"
#include "src/raft/raft_service.hpp"

using ::fver::base::NonCopyable;

namespace fver {

namespace raft {

struct RaftServerConfig;

class RaftServer {
 public:



 private:

  RaftServiceImpl raft_service_impl_;
  RaftNode raft_node_;
};




}  // namespace raft

}  // namespace fver

#endif