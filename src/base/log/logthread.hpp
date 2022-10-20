#ifndef SRC_BASE_LOG_LOGTHREAD_H_
#define SRC_BASE_LOG_LOGTHREAD_H_

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <system_error>
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
  void AddLogger(std::shared_ptr<Logger> log);
  void Init(const std::string& logpath, const int lev, const bool isSync,
            const std::string& logPrename);
  void Stop();
  ~LogThread();

 private:
  int loglev_;
  LogFile file_;
  std::mutex mtx_;
  std::vector<std::shared_ptr<Logger>> logVectmp_;
  std::vector<std::shared_ptr<Logger>> logworkers_;
  std::thread syncThread_;
  bool isSync_;
  bool isRunning_;

  // for Logger Thread
  std::shared_ptr<std::condition_variable> cond_;
  std::shared_ptr<std::atomic<uint64_t>> sumWrites_;
  std::mutex logMutex_;
};

}  // namespace log
}  // namespace base
}  // namespace fver

#endif