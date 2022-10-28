#ifndef SRC_RAFT_LOG_H_
#define SRC_RAFT_LOG_H_

#include <cstdint>
#include <string>
#include <vector>

#include "src/base/noncopyable.hpp"

using ::fver::base::NonCopyable;

namespace fver {

namespace raft {

struct LogEntry {
  std::string command;
  uint64_t term;
  uint64_t index;
};

class RaftLog : public NonCopyable {
 public:

  
 private:
  // 已知的 最大提交下标
  uint64_t LeaderCommitIndex_;
  // 远程节点的 下一个应该发送的日志下标
  uint64_t log_NextIndex_;

  // 远程节点的 下一个应该同步的日志下标
  uint64_t log_MatchIndex_;

  // 当前已经应用到状态机的日志下标
  uint64_t LastAppliedIndex_;
  std::vector<LogEntry> logEntry_;
};

}  // namespace raft

}  // namespace fver

#endif