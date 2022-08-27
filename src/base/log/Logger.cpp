#include "Logger.hpp"

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
  prebuf_.Expansion(bufsize);
  tailbuf_.Expansion(bufsize);
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
  logthread->PushLogWorker(this);
  this->logway_ = logthread->GetLogWay();
  if (logway_ == AsyncLogThread::kLogToFile) {
    SyncToFile();
  }
  curloglevel_ = static_cast<LogLevel>(logthread->GetLogLevel());
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