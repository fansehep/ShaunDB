#include "Logger.hpp"

#include <assert.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <stdarg.h>

namespace fver::base::log {

Logger::Logger()
    : prebuf_(0),
      tailbuf_(0),
      curloglevel_(LogLevel::kInfo),
      logway_(AsyncLogThread::kLogStdOut) {
  curbufptr_ = &prebuf_;
}

void Logger::SetCurrentLogLevel(LogLevel lev) { curloglevel_ = lev; }

void Logger::SetCurrentLogBufferSize(uint32_t bufsize) {
  assert(prebuf_.Expansion(bufsize));
  assert(tailbuf_.Expansion(bufsize));
  curbufptr_ = &prebuf_;
}

LogBuffer* Logger::GetLogBufferPtr() {
  std::lock_guard<std::mutex> lgk(mtx_);
  return curbufptr_ == &prebuf_ ? &tailbuf_ : &prebuf_;
}

bool Logger::IsSync() { return curbufptr_->IsSync(); }

Logger::Logger(AsyncLogThread* logthread)
    : prebuf_(0),
      tailbuf_(0),
      logway_(AsyncLogThread::kLogStdOut),
      threadid_(pthread_self()) {
  if (!logthread) {
    return;
  }
  this->logway_ = logthread->GetLogWay();
  if (logway_ == AsyncLogThread::kLogToFile) {
    SyncToFile();
  }
  curloglevel_ = static_cast<LogLevel>(logthread->GetLogLevel());
  logthread->PushLogWorker(shared_from_this());
}

Logger::Logger(const std::unique_ptr<AsyncLogThread>& thd)
    : prebuf_(0),
      tailbuf_(0),
      logway_(AsyncLogThread::kLogStdOut),
      threadid_(pthread_self()),
      unlogtimes_(0),
      curbufptr_(&prebuf_) {
#ifdef DEBUG
  printf("Logger Push the AsyncLogThread");
#endif
  if (!thd.get()) {
    printf("AsyncLogThread is nullptr\n");
    return;
  }
  this->logway_ = thd->GetLogWay();
  SyncToFile();
  curloglevel_ = static_cast<LogLevel>(thd->GetLogLevel());
  thd->PushLogWorker(shared_from_this());
}

void Logger::SyncToFile() {
  logway_ = AsyncLogThread::kLogToFile;
  prebuf_.Expansion(LogBuffer::kMidBufferSize);
  tailbuf_.Expansion(LogBuffer::kMidBufferSize);
  curbufptr_ = &prebuf_;
}

void Logger::SetBufferHorSize(double horsize) {
  bufhorsize_ = horsize;
  prebuf_.SetBufHorSize(bufhorsize_);
  tailbuf_.SetBufHorSize(bufhorsize_);
}

bool Logger::IsChangeBuffer() { return curbufptr_->IsChangeBuffer(); }

void Logger::AddLogTimes() { unlogtimes_++; }

void Logger::ClearLogTimes() { unlogtimes_ = 0; }

bool Logger::IsLogToDisk() {
  if (unlogtimes_ >= 10) {
    return true;
  }
  return false;
}

void Logger::ChangeBufferPtr() {
  std::lock_guard<std::mutex> lgk(mtx_);
  curbufptr_ = (curbufptr_ == &prebuf_) ? &tailbuf_ : &prebuf_;
}

}  // namespace fver::base::log