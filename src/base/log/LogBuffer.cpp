#include "LogBuffer.hpp"

#include <fmt/format.h>


#include <cstring>

namespace fver::base::log {

LogBuffer::LogBuffer(uint32_t maxsize)
    : maxsize_(maxsize), bufferptr_(nullptr), cursize_(0) {
  bufferptr_ = new char(maxsize_);
  if (nullptr == bufferptr_) {
    exit(-1);
  }
}

LogBuffer::LogBuffer()
    : maxsize_(kSmallBufferSize), bufferptr_(nullptr), cursize_(0) {
  bufferptr_ = new char(maxsize_);
  if (nullptr == bufferptr_) {
    exit(-1);
  }
}

LogBuffer::~LogBuffer() {
  if (bufferptr_) {
    delete bufferptr_;
  }
}

bool LogBuffer::Clear() {
  std::memset(bufferptr_, 0, maxsize_);
  cursize_ = 0;
  return true;
}

bool LogBuffer::Push(const std::string& str) {
  // 当日志过多时, 丢弃以前的日志.
  if (maxsize_ - cursize_ < str.size()) [[unlikely]] {
    Clear();
  }
  std::memcpy(bufferptr_ + cursize_, str.c_str(), str.size());
  cursize_ += str.size();
  return true;
}

bool LogBuffer::Push(const char* str, uint32_t len) {
  if (maxsize_ - cursize_ < len) [[unlikely]] {
    Clear();
  }
  std::memcpy(bufferptr_ + cursize_, str, len);
  cursize_ += len;
  return true;
}

bool LogBuffer::Push(const char* str) {
  int len = strlen(str);
  if (maxsize_ - cursize_ < static_cast<uint32_t>(len)) [[unlikely]] {
    Clear();
  }
  std::memcpy(bufferptr_ + cursize_, str, len);
  cursize_ += len;
  return true;
}

bool LogBuffer::Expansion(uint32_t bufsize) {
  if (bufferptr_) {
    delete bufferptr_;
  }
  bufferptr_ = new char(bufsize);
  if (bufferptr_ == nullptr) {
    auto now = fver::base::TimeStamp::Now().ToFormatTodayNowMs();
    fmt::print("{} {} {} new error!\n", now.c_str(), __FILE__, __LINE__);
  }
  cursize_ = 0;
  maxsize_ = bufsize;
  return true;
}

char* LogBuffer::GetBufferPtr() { return bufferptr_; }

uint32_t LogBuffer::GetCurrentSize() { return cursize_; }

uint32_t LogBuffer::GetMaxSize() { return maxsize_; }

}  // namespace fver::base::log