#ifndef SRC_BASE_LOG_LOGTHREAD_H_
#define SRC_BASE_LOG_LOGTHREAD_H_

#include <mutex>
#include <thread>
#include <vector>
#include <condition_variable>
#include <memory>

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
};

}  // namespace log
}  // namespace base
}  // namespace fver

#endif