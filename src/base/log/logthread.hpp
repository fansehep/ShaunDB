#ifndef SRC_BASE_LOG_LOGTHREAD_H_
#define SRC_BASE_LOG_LOGTHREAD_H_

#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "src/base/log/logfile.hpp"

namespace fver {
namespace base {
namespace log {

class Logger;

class LogThread {
 public:
  LogThread();
  void AddLogger(Logger* log);
  void Init(const std::string& logpath, const int lev);
  void Stop();
  ~LogThread();

 private:
  int loglev_;
  LogFile file_;
  std::mutex mtx_;
  std::vector<Logger*> logVectmp_;
  std::vector<Logger*> logworkers_;
  std::thread syncThread_;
  bool isSync_;
  bool isRunning_;
  // 是否在新的一轮 Sync 中加入日志
  bool isAdd_;
  // 根据 writePerLoop_ 来决定每秒睡眠的时间
  uint64_t sleepPerLoopMs_;
  // 活跃的 Logger shu li a
  uint64_t activeLoggern_;
  // 最新一轮中的 sync 到文件的 buf 的总量
  uint64_t writePerLoop_;
};

}  // namespace log
}  // namespace base
}  // namespace fver

#endif