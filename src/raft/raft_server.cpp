#include "src/raft/raft_server.hpp"

#include <boost/fiber/future/future.hpp>
#include <boost/fiber/future/promise.hpp>
#include <functional>
#include <memory>

#include "src/base/log/logging.hpp"
#include "src/net/repeated_timer.hpp"
#include "src/raft/fiber_define.hpp"
#include "src/raft/raft_peer.hpp"
#include "src/raft/raftconf.hpp"

namespace fver {

namespace raft {

bool RaftServer::Init(const std::shared_ptr<net::NetServer> &net_server,
                      const RaftConfig &config) {
  //
  struct timeval requestVoteTimer_timeval;
  RequestVoteTimer_.Init(
      requestVoteTimer_timeval,
      std::bind(&RaftServer::_RequestVoteOnTimed, this, std::placeholders::_1),
      net_server, "raft_request_vote");
  struct timeval appendEntriesTimer_timeval;
  AppendEntriesTimer_.Init(appendEntriesTimer_timeval,
                           std::bind(&RaftServer::_AppendEntriesOnTimed, this,
                                     std::placeholders::_1),
                           net_server, "raft_append_entries");
}

int RaftServer::_RequestVoteOnTimed(net::RepeatedTimer *timer) {
  if (raft_node_.is_timeout() == false) {
    return 0;
  }
  // 此时已经超时
  // 成为 Candidates
  raft_node_.be_Candidates();
  // term += 1
  raft_node_.addTerm();
  LOG_TRACE("server ip: {} begin vote term: {}", raft_node_.get_NodeIp(),
            raft_node_.getTerm());
  go([&]() {
    uint32_t i = 0;
    // 当前投我的总票数
    std::atomic<uint32_t> vote_counts = 1;
    std::atomic<bool> is_leader = false;
    std::vector<
        std::shared_ptr<boost::fibers::promise<RaftMes::RequestVoteReply>>>
        request_info_vec(RaftConfig::node_counts);
    //
    for (auto &iter : request_info_vec) {
      iter =
          std::make_shared<boost::fibers::promise<RaftMes::RequestVoteReply>>();
    }
    // 并发发送 RPC
    for (auto &peer_node_server : peerNodeVec_) {
      go([&, i]() {
        assert(i >= 0 && i <= peerNodeVec_.size());
        peer_node_server.RequestVote(this->raft_node_, request_info_vec[i]);
      }).detach();
      i++;
    }
    for (auto &iter : request_info_vec) {
      go([&]() -> void {
        auto future_reply = iter->get_future();
        //
        future_reply.wait_for(
            std::chrono::seconds(RaftConfig::min_election_interval_ms));
        // RPC 成功
        if (true == future_reply.valid()) {
          auto reply = future_reply.get();
          // 如果发送某一个节点的 term > 当前节点的term
          if (reply.term() > raft_node_.getTerm()) {
            // 成为 Follower
            LOG_INFO("server ip: {} cur term: {} reply term: {} be follower",
                     raft_node_.get_NodeIp(), raft_node_.getTerm(),
                     reply.term());
            raft_node_.be_Follower();
            raft_node_.setTerm(reply.term());
          }
          // 投票成功
          if (true == reply.votegranted()) {
            vote_counts++;
          }
          // 如果收到了集群上 1/2 的节点投票
          // 就成为 Leader
          if (vote_counts > (RaftConfig::node_counts / 2) &&
              false == is_leader) {
            is_leader = true;
            raft_node_.be_Leader();
            LOG_TRACE("server ip: {} vote be Leader", raft_node_.get_NodeIp());
          }
        }
      }).detach();
    }
  }).detach();
  return 0;
}

int RaftServer::_AppendEntriesOnTimed(net::RepeatedTimer *timer) {}

}  // namespace raft

}  // namespace fver