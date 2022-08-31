#include "AsyncLoggingThread.hpp"

#include <chrono>
#include <functional>
#include <string_view>

#include "Logger.hpp"
#include "ThreadLocalBuffer.hpp"

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
      logthread_(nullptr) {
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
  logthread_ = new std::thread([this]() {
    while (stop_) {
      // 1. 简单的sleep_for
      // 2. 后续可以改进为 conditionvariable 让AsyncLogThread 阻塞
      // 3. 当 Logger 需要写日志时 cond = true
      //  std::this_thread::sleep_for(std::chrono::milliseconds(0));
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
#ifdef DEBUG
        fmt::print("{} {} swap tmpworkers\n", __FILE__, __LINE__);
#endif
      }
#ifdef DEBUG
      fmt::print("{} {} cur logger nums = {} tmpworkers = {} \n", __FILE__,
                 __LINE__, logworkers_.size(), tmpworkers_.size());
#endif
      const int N = logworkers_.size();
      for (int i = 0; i < N; ++i) {
        auto thrd = logworkers_[i];
        // 1. buf 到达了阈值需要刷盘
        // 2. 时间到了 10 * 100 = 1s 必须刷盘
        // 3. asynglogthread_ 需要被析构，此时必须刷所有buf
        if (thrd->logptr_->IsTimeOut() || thrd->logptr_->IsFillThresold() ||
            !stop_) {
#ifdef DEBUG
          fmt::print(
              "{} {} cur logger nums = {} tmpworkers = {} ready to write\n",
              __FILE__, __LINE__, logworkers_.size(), tmpworkers_.size());
#endif
          // 1. 改变当前线程正在写的 buf (thread safe)
          thrd->logptr_->ChangeBufferPtr();
          // 2. 写入被换下来的 buf
          file_.Write(thrd->logptr_->GetBufPtr(), thrd->logptr_->GetBufSize());
          // 3. 清空被换下来的 buf
          thrd->logptr_->ClearTmpBuf();
          // 4. 重置单个 logger 超时时间
          thrd->logptr_->ClearLogTimes();
        } else {
          // 5. 当前没有写入, 超时时间 +1
#ifdef DEBUG
          fmt::print("{} {} add logtimes istimeout: {}\n", __FILE__, __LINE__,
                     thrd->logptr_->IsTimeOut());
#endif
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
#ifdef DEBUG
  fmt::print("{} {} AsyncLogThread destructor!\n", __FILE__, __LINE__);
#endif
  if (logthread_) {
    if (logthread_->joinable()) {
#ifdef DEBUG
      fmt::print("AsyncLogThread destructor!\n");
#endif
      logthread_->join();
    }
  }
}

}  // namespace fver::base::log