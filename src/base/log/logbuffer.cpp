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