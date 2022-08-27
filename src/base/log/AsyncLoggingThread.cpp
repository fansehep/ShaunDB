#include "AsyncLoggingThread.hpp"

#include <chrono>
#include <functional>

#include "Logger.hpp"

namespace fver::base::log {

inline void LogToFile(LogFile& file, const std::string& logment) {
  file.Write(logment);
}

AsyncLogThread::AsyncLogThread(LogWay logway, const std::string& logpath)
    : logway_(logway), file_(logpath), loglevel_(0), stop_(true) {
  // make the log to file
}

AsyncLogThread::AsyncLogThread() : logway_(kLogStdOut), file_(), stop_(true) {}

void AsyncLogThread::PushLogWorker(const Logger* lger) {
  std::lock_guard<std::mutex> lgk(this->mtx_);
  logworkers_.push_back(const_cast<Logger*>(lger));
}

void AsyncLogThread::Init(const std::string& logpath,
                          const uint32_t threadlocalbufsize, const int loglev,
                          const double bufhosize) {
  logway_ = AsyncLogThread::kLogToFile;
  file_.SetLogPath(logpath);
  bufsize_ = threadlocalbufsize;
  loglevel_ = loglev;
  bufhorizontalsize_ = bufhosize;
  for (auto thrd : logworkers_) {
    thrd->SetCurrentLogLevel(static_cast<Logger::LogLevel>(loglevel_));
    thrd->SetCurrentLogBufferSize(bufsize_);
    thrd->SetBufferHorSize(bufhosize);
  }
  logthread_ = std::make_unique<std::thread>([&]() {
    while (stop_) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      for (auto& thrd : logworkers_) {
        if (thrd->IsSync() || thrd->IsLogToDisk()) {
          // 这里属于被动必须刷盘
          if (thrd->IsLogToDisk()) {
            thrd->ChangeBufferPtr();
          }
          file_.Write(thrd->GetLogBufferPtr()->GetBufferPtr(),
                      thrd->GetLogBufferPtr()->GetCurrentSize());
          thrd->GetLogBufferPtr()->Clear();
          // 超过 10 * 100 = 1s 默认必须刷盘
          thrd->ClearLogTimes();
        } else {
          thrd->AddLogTimes();
        }
      }
    }
  });
}

AsyncLogThread::~AsyncLogThread() {
  stop_ = false;
  // if (logthread_->joinable()) {
  //   logthread_->join();
  // }
  if (logthread_.get()) {
    if (logthread_->joinable()) {
      logthread_->join();
    }
  }
}

}  // namespace fver::base::log