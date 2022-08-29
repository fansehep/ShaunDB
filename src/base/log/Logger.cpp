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
      logway_(AsyncLogThread::kLogStdOut),
      threadid_(pthread_self()) {
#ifdef DEBUG
  fmt::print("thid: {} logger construct\n", threadid_);
#endif
      curbufptr_ = &prebuf_;
}

Logger::Logger(uint32_t bufsize)
    : prebuf_(bufsize),
      tailbuf_(bufsize),
      curloglevel_(LogLevel::kInfo),
      logway_(AsyncLogThread::kLogToFile),
      threadid_(pthread_self()),
      unlogtimes_(0),
      time_() {
#ifdef DEBUG
  fmt::print("Logger Constructor! LogWay: {}\n", logway_);
#endif
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

void Logger::SetLogToFile() {
  logway_ = AsyncLogThread::kLogToFile;
}

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

void Logger::SetLogWay(AsyncLogThread::LogWay lw) {
  logway_ = lw;
}



}  // namespace fver::base::log