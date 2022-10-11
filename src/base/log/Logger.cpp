#include "src/base/log/Logger.hpp"

#include <assert.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <stdarg.h>

namespace fver::base::log {

Logger::Logger()
    : buf_(nullptr),
      curloglevel_(LogLevel::kInfo),
      logway_(AsyncLogThread::kLogStdOut),
      threadid_(pthread_self()) {
#ifdef DEBUG
  fmt::print("thid: {} logger construct\n", threadid_);
#endif
}

Logger::Logger(uint32_t bufsize)
    : buf_(std::make_shared<ThreadLocalBuffer>(bufsize)),
      curloglevel_(LogLevel::kInfo),
      logway_(AsyncLogThread::kLogToFile),
      threadid_(pthread_self()),
      time_() {
#ifdef DEBUG
  fmt::print("Logger Constructor! LogWay: {}\n", logway_);
#endif
}

void Logger::SetCurrentLogLevel(LogLevel lev) { curloglevel_ = lev; }

bool Logger::IsChangeBuffer() { return buf_->IsChangeBufPtr(); }

void Logger::ChangeBufferPtr() { buf_->ChangeBufPtr(); }

void Logger::AddLogTimes() { buf_->AddUnlogTimes(); }

void Logger::ClearLogTimes() { buf_->ClearUnlogTimes(); }

void Logger::SetLogToFile() { logway_ = AsyncLogThread::kLogToFile; }

void Logger::SetLogToStdOut() { logway_ = AsyncLogThread::kLogStdOut; }


bool Logger::IsTimeOut() {
  return buf_->IsChangeBufPtr();
}

bool Logger::IsFillThresold() {
  return buf_->IsFillThresold();
}
}  // namespace fver::base::log