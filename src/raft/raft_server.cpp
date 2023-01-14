#include "src/raft/raft_server.hpp"

#include <boost/fiber/future/future.hpp>
#include <boost/fiber/future/promise.hpp>
#include <functional>
#include <memory>

#include "src/net/repeated_timer.hpp"
#include "src/raft/fiber_define.hpp"
#include "src/raft/raft_peer.hpp"

namespace fver {

namespace raft {

bool RaftServer::Init(const std::shared_ptr<net::NetServer> &net_server,
                      const RaftServerConfig &config) {
  //
  struct timeval requestVoteTimer_timeval;
  RequestVoteTimer_.Init(
      requestVoteTimer_timeval,
      std::bind(&RaftServer::_RequestVoteOnTimed, this, std::placeholders::_1),
      net_server, "request_vote");
  struct timeval appendEntriesTimer_timeval;
  AppendEntriesTimer_.Init(appendEntriesTimer_timeval,
                           std::bind(&RaftServer::_AppendEntriesOnTimed, this,
                                     std::placeholders::_1),
                           net_server, "append_entries");
}

int RaftServer::_RequestVoteOnTimed(net::RepeatedTimer *timer) {
  go([&]() {
    uint32_t i = 0;
    //
    std::vector<
        std::shared_ptr<boost::fibers::promise<RaftMes::RequestVoteReply>>>
        request_info_vec;
    //
    for (auto &iter : request_info_vec) {
      iter =
          std::make_shared<boost::fibers::promise<RaftMes::RequestVoteReply>>();
    }
    //
    for (auto &peer_node_server : peerNodeVec_) {
      go([&, i]() {
        assert(i >= 0 && i <= peerNodeVec_.size());
        peer_node_server.RequestVote(this->raft_node_, request_info_vec[i]);
      }).detach();
      i++;
    }
    for (auto &iter : request_info_vec) {
      go([&]() -> void {
        boost::fibers::future<RaftMes::RequestVoteReply> future_reply =
            iter->get_future();
        //
        future_reply.wait_for(std::chrono::seconds());
      }).detach();
    }
  }).detach();
  return 0;
}

int RaftServer::_AppendEntriesOnTimed(net::RepeatedTimer *timer) {}

}  // namespace raft

}  // namespace fver