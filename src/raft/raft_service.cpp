#include "src/raft/raft_service.hpp"
#include "src/raft/raft_node.hpp"
#include <grpcpp/support/status.h>

namespace fver {

namespace raft {

RaftServiceImpl::RaftServiceImpl() : node_(nullptr) {}

void RaftServiceImpl::Init(RaftNode* node) {
  assert(nullptr != node);
  node_ = node;
}


grpc::Status RaftServiceImpl::RequestVote(
    grpc::ServerContext* context,
    const RaftMes::RequestVoteArgs* arg,
    RaftMes::RequestVoteReply* reply) {
  // 如果 peer node.term < 当前的节点的 term
  // 拒绝投票
  if (arg->term() < node_->getTerm()) {
    reply->set_term(node_->getTerm());
    reply->set_votegranted(false);
    return grpc::Status::OK;
  }
  if (arg->term() > node_->getTerm()) {
    node_->be_Follower();
    node_->setTerm(arg->term());
    return grpc::Status::OK;
  }
  
}

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