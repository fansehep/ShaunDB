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

LogBuffer::LogBuffer() : head_(20480), tail_(20480), curPtr_(nullptr) {}

bool LogBuffer::Push(const std::string& logment) {
  std::lock_guard<std::mutex> lg(mtx_);
  // 超过水平线, 就直接丢失
  if (curPtr_->buflen_ - curPtr_->offset_ < logment.size()) {
    return false;
  }
  std::memcpy(curPtr_->bufptr_ + curPtr_->offset_, logment.data(),
              logment.size());
  curPtr_->offset_ += static_cast<const uint32_t>(logment.size());
  return true;
}

Buffer* LogBuffer::SwapBuffer() {
  curPtr_ = (curPtr_ == &head_ ? &tail_ : &head_);
  return (curPtr_ == &head_ ? &tail_ : &head_);
}

void LogBuffer::ClearOffset(Buffer* buf) {
  std::lock_guard<std::mutex> lg(mtx_);
  buf->offset_ = 0;
}

std::mutex* LogBuffer::getMutex() { return &mtx_; }

VecLogBuffer::VecLogBuffer() {}

VecLogBuffer::VecLogBuffer(uint32_t buffer_size) {
  head_.reserve(buffer_size);
  head_.reserve(buffer_size);
  curPtr_ = &head_;
}

std::mutex* VecLogBuffer::getMutex() { return &mtx_; }

std::vector<char>* VecLogBuffer::SwapBuffer() {
  curPtr_ = (curPtr_ == &head_ ? &tail_ : &head_);
  return (curPtr_ == &head_ ? &tail_ : &head_);
}

}  // namespace log

}  // namespace base

}  // namespace fver