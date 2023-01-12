#include "src/raft/raft_service.hpp"

#include <grpcpp/support/status.h>

namespace fver {

namespace raft {

grpc::Status RaftServiceImpl::RequestVote(
    grpc::ServerContext* context,
    const RaftMes::RequestVoteArgs* request_vote_arg,
    RaftMes::RequestVoteReply* request_vote_reply) {}

grpc::Status RaftServiceImpl::AppendEntries(
    grpc::ServerContext* context,
    const RaftMes::AppendEntriesArgs* append_entries_arg,
    RaftMes::AppendEntriesReply* append_entries_reply) {}

grpc::Status RaftServiceImpl::InstallSnapshot(
    grpc::ServerContext* context,
    const RaftMes::InstallSnapshotArgs* install_snapshot_arg,
    RaftMes::InstallSnapshotReply* install_snapshot_reply) {}




}  // namespace raft

}  // namespace fver