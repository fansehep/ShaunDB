#include "src/raft/raft_peer.hpp"

#include "fmt/format.h"
#include <grpcpp/client_context.h>
#include <grpcpp/security/credentials.h>

#include <future>

#include "src/base/log/logging.hpp"
#include "src/raft/raft_node.hpp"

namespace fver {

namespace raft {

bool RaftPeerNode::Init(const std::string &peer_ip, const uint32_t peer_port) {
  server_info_ = fmt::format("{}:{}", peer_ip, peer_port);
  LOG_INFO("init peer server info: {}", server_info_);
  this->peer_stub_ = RaftMes::RaftService::NewStub(
      grpc::CreateChannel(server_info_, grpc::InsecureChannelCredentials()));
  if (nullptr == this->peer_stub_.get()) {
    assert(false);
    LOG_WARN("init peer server info: {} error!");
    return false;
  }
  return true;
}

void RaftPeerNode::RequestVote(
    const RaftNode &node,
    const std::shared_ptr<boost::fibers::promise<RaftMes::RequestVoteReply>>
        &request_vote_reply_promise) const {
  // TODO need a object pool? like boost::object_pool ?
  RaftMes::RequestVoteArgs request_vote_args;
  RaftMes::RequestVoteReply request_vote_reply;
  grpc::ClientContext client_context;
  request_vote_args.set_term(node.getTerm());
  request_vote_args.set_candidateid(node.getID());
  request_vote_args.set_lastlogindex(node.getLog()->getLastLogIndex());
  request_vote_args.set_lastlogterm(node.getLog()->getLastLogTerm());
  auto status = peer_stub_->RequestVote(&client_context, request_vote_args,
                                        &request_vote_reply);
  if (true == status.ok()) {
    request_vote_reply_promise->set_value(std::move(request_vote_reply));
  } else {
    LOG_WARN("requestvote to {} error, {}", server_info_,
             status.error_message());
  }
}

}  // namespace raft

}  // namespace fver