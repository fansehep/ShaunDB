#include "ThreadLocalBuffer.hpp"

namespace fver::base::log {

ThreadLocalBuffer::ThreadLocalBuffer(uint32_t size)
    : prebuf_(size), tailbuf_(size), mtx_(), unlogtimes_(0), bufhorsize_(0.8) {
  curbufptr_ = &prebuf_;
  tmpbufptr_ = &tailbuf_;
}

ThreadLocalBuffer::ThreadLocalBuffer()
    : prebuf_(0), tailbuf_(0), mtx_(), unlogtimes_(0), bufhorsize_(0.8) {}

void ThreadLocalBuffer::Push(const std::string& buf) {
  std::lock_guard<std::mutex> lgk(mtx_);
  curbufptr_->Push(buf);
}

void ThreadLocalBuffer::Push(const char* buf) {
  std::lock_guard<std::mutex> lgk(mtx_);
  curbufptr_->Push(buf);
}

void ThreadLocalBuffer::Push(char* buf, int len) {
  std::lock_guard<std::mutex> lgk(mtx_);
  curbufptr_->Push(buf, len);
}

void ThreadLocalBuffer::ChangeBufPtr() {
  std::lock_guard<std::mutex> lgk(mtx_);
  curbufptr_ = (curbufptr_ == &prebuf_ ? &tailbuf_ : &prebuf_);
  tmpbufptr_ = (curbufptr_ == &prebuf_ ? &tailbuf_ : &prebuf_);
}

char* ThreadLocalBuffer::GetBeginPtr() { return tmpbufptr_->GetBufferPtr(); }

void ThreadLocalBuffer::ClearTmpBuf() {
  std::lock_guard<std::mutex> lgk(mtx_);
  tmpbufptr_->Clear();
}

uint32_t ThreadLocalBuffer::GetSize() { return tmpbufptr_->GetCurrentSize(); }

bool ThreadLocalBuffer::IsChangeBufPtr() {
  std::lock_guard<std::mutex> lgk(mtx_);
  return unlogtimes_ >= 10;
}

void ThreadLocalBuffer::AddUnlogTimes() { unlogtimes_++; }

void ThreadLocalBuffer::ClearUnlogTimes() { unlogtimes_ = 0; }

int ThreadLocalBuffer::GetUnlogTimes() { return unlogtimes_; }
bool ThreadLocalBuffer::IsFillThresold() {
  return (static_cast<double>(curbufptr_->GetCurrentSize()) /
          static_cast<double>(curbufptr_->GetMaxSize())) >= bufhorsize_;
}

void ThreadLocalBuffer::SetBufhorSize(double hor) { bufhorsize_ = hor; }

double ThreadLocalBuffer::GetBufhorSize() { return bufhorsize_; }

}  // namespace fver::base::log
