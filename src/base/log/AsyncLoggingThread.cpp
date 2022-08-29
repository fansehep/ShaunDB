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

AsyncLogThread::AsyncLogThread()
    : logway_(kLogStdOut),
      file_(),
      stop_(true),
      logworkers_(),
      tmpworkers_(),
      logthread_() {
#ifdef DEBUG
  printf("AsyncLogThread Constructor!\n");
#endif
}

void AsyncLogThread::PushLogWorker(LoggerImp* lger) {
  std::lock_guard<std::mutex> lgk(mtx_);
  lger->logptr_->SetLogToFile();
  tmpworkers_.push_back(lger);
}

void AsyncLogThread::Init(const std::string& logpath,
                          const uint32_t threadlocalbufsize, const int loglev,
                          const double bufhosize) {
  stop_ = true;
  logway_ = AsyncLogThread::kLogToFile;
  file_.SetLogPath(logpath);
  bufsize_ = threadlocalbufsize;
  loglevel_ = loglev;
  bufhorizontalsize_ = bufhosize;
#ifdef DEBUG
  fmt::print("{} {} cur logger nums = {} tmpworkers = {} \n", __FILE__,
             __LINE__, logworkers_.size(), tmpworkers_.size());
#endif
  logthread_ = std::make_unique<std::thread>([this]() {
    while (stop_) {
      // 1. 简单的sleep_for
      // 2. 后续可以改进为 conditionvariable 让AsyncLogThread 阻塞
      // 3. 当 Logger 需要写日志时 cond = true
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      // tmpworkers_ 为了其他新创建的线程当有新的 LoggerImp 创建加入时
      // 先加入到 tmpworkers_ 然后在遍历的时候,将他们 push 到 logworkers_
      // 1. 有效减少锁的范围
#ifdef DEBUG
      fmt::print("{} {} cur logger nums = {} tmpworkers = {} \n", __FILE__,
                 __LINE__, logworkers_.size(), tmpworkers_.size());
#endif
      if (!tmpworkers_.empty()) {
        std::lock_guard<std::mutex> mtx(this->mtx_);
        logworkers_.insert(logworkers_.end(), tmpworkers_.begin(),
                           tmpworkers_.end());
        tmpworkers_.clear();
      }
      for (auto& thrd : logworkers_) {
        // 1. buf 到达了阈值需要刷盘
        // 2. 时间到了 10 * 100 = 1s 必须刷盘
        // 3. asynglogthread_ 需要被析构，此时必须刷所有buf
        if (thrd->logptr_->IsSync() || thrd->logptr_->IsLogToDisk() || !stop_) {
          // 这里属于被动必须刷盘
          // 当前正在写的buf 达到了阈值, 默认是 0.8
          // 单个 LoggerImp 拥有两个 buf, 达到阈值后, 立即交换buf
          // 有效减少锁的时间
          if (thrd->logptr_->IsChangeBuffer() || !stop_) {
            thrd->logptr_->ChangeBufferPtr();
          }
          file_.Write(thrd->logptr_->GetLogBufferPtr()->GetBufferPtr(),
                      thrd->logptr_->GetLogBufferPtr()->GetCurrentSize());
#ifdef DEBUG
          const std::string logtemp(
              thrd->logptr_->GetLogBufferPtr()->GetBufferPtr(),
              thrd->logptr_->GetLogBufferPtr()->GetCurrentSize());
          fmt::print("{} {} buf: {}\n", __FILE__, __LINE__, logtemp);
#endif
          thrd->logptr_->GetLogBufferPtr()->Clear();
          // 超过 10 * 100 = 1s 默认必须刷盘
          thrd->logptr_->ClearLogTimes();
        } else {
          thrd->logptr_->AddLogTimes();
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
#ifdef DEBUG
      fmt::print("AsyncLogThread destructor!\n");
#endif
      logthread_->join();
    }
  }
}

}  // namespace fver::base::log