#include "src/base/log/logbuffer.hpp"

#include <fmt/format.h>

#include <condition_variable>
#include <cstring>
#include <memory>
#include <mutex>

#include "src/base/log/logbuffer.hpp"

namespace fver {

namespace base {

namespace log {

LogBuffer::LogBuffer(uint32_t bufferSize)
    : head_(bufferSize), tail_(bufferSize), curPtr_(nullptr) {
  curPtr_ = &head_;
}

LogBuffer::LogBuffer() : head_(1024), tail_(1024), curPtr_(nullptr) {}

bool LogBuffer::Push(const std::string& logment) {
  std::lock_guard<std::mutex> lg(mtx_);
  std::memcpy(curPtr_->bufptr_ + curPtr_->offset_, logment.data(),
              logment.size());
  curPtr_->offset_ += static_cast<const uint32_t>(logment.size());
  return true;
}

Buffer* LogBuffer::SwapBuffer() {
  std::lock_guard<std::mutex> lg(mtx_);
  curPtr_ = (curPtr_ == &head_ ? &tail_ : &head_);
  return (curPtr_ == &head_ ? &tail_ : &head_);
}

void LogBuffer::ClearOffset(Buffer* buf) {
  std::lock_guard<std::mutex> lg(mtx_);
  buf->offset_ = 0;
}


}  // namespace log

}  // namespace base

}  // namespace fver