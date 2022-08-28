#ifndef SRC_BASE_LOG_ASYNCLOGGINGTHREAD_H_
#define SRC_BASE_LOG_ASYNCLOGGINGTHREAD_H_

#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "LogFile.hpp"

namespace fver::base::log {

class Logger;

class AsyncLogThread : public NonCopyable {
 public:
  friend void TraverseLogger(AsyncLogThread* logthrad);
  enum LogWay {
    kLogStdOut,
    kLogToFile,
  };
  void Init(const std::string& logpath, const uint32_t threadlocalbufsize,
            const int loglev, const double bufhosize);
  AsyncLogThread();
  ~AsyncLogThread();
  AsyncLogThread(LogWay logway, const std::string& logpath);
  void PushLogWorker(const std::shared_ptr<Logger> lger);
  LogWay GetLogWay() { return logway_; }
  void SetLogLevel(int loglv) { loglevel_ = loglv; }
  int GetLogLevel() { return loglevel_; }
  void SetBufSize(uint32_t bufsize) { bufsize_ = bufsize; }
  uint32_t GetBufSize() { return bufsize_; }
  double GetBufHorizontalSize() { return bufhorizontalsize_; }

 private:
  void SetLogPath(const std::string& logpath);
  void SetLogWay(LogWay lw);
  std::mutex mtx_;
  LogWay logway_;
  uint32_t bufsize_;
  double bufhorizontalsize_;
  int loglevel_;
  std::vector<std::shared_ptr<Logger>> logworkers_;
  // 当程序创建新的线程时, 此时的thread_local Logger 要被 AsyncLoggingThread 所管理
  // 所以将新创建的 Logger push 到 tmpworkers_ 在 AsyncLoggingThread 遍历的时候
  // 使用很短的锁区间 即可达到线程安全
  std::vector<std::shared_ptr<Logger>> tmpworkers_;
  std::unique_ptr<std::thread> logthread_;
  LogFile file_;
  bool stop_;
};

}  // namespace fver::base::log

#endif