#ifndef SRC_RAFT_RAFT_SERVICE_H_
#define SRC_RAFT_RAFT_SERVICE_H_

#include <grpcpp/grpcpp.h>
#include <grpcpp/server_context.h>
#include <grpcpp/support/status.h>

#include "src/base/noncopyable.hpp"
#include "src/raft/raft.grpc.pb.h"

namespace fver {

namespace raft {

class RaftServiceImpl final : public RaftMes::RaftService::Service,
                              public fver::base::NonCopyable {
 public:
  grpc::Status RequestVote(grpc::ServerContext*,
                           const RaftMes::RequestVoteArgs*,
                           RaftMes::RequestVoteReply*) override;

  grpc::Status AppendEntries(grpc::ServerContext*,
                             const RaftMes::AppendEntriesArgs*,
                             RaftMes::AppendEntriesReply*) override;
  /**
   * @brief
   *
   */
  grpc::Status InstallSnapshot(grpc::ServerContext*,
                               const RaftMes::InstallSnapshotArgs*,
                               RaftMes::InstallSnapshotReply*) override;

  RaftServiceImpl() = default;
  ~RaftServiceImpl() = default;
};

}  // namespace raft

}  // namespace fver

#endif