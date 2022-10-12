#ifndef SRC_BASE_LOG_LOGBUFFER_H_
#define SRC_BASE_LOG_LOGBUFFER_H_

#include <string.h>

#include <condition_variable>
#include <memory>
#include <mutex>
#include <ostream>
#include <string>

namespace fver {

namespace base {

namespace log {

class LogFile;
static constexpr double kBufferHorSize = 0.7;

struct Buffer {
  char* bufptr_;
  uint32_t offset_;
  uint32_t buflen_;
  Buffer() : bufptr_(nullptr), offset_(0), buflen_(0) {}
  Buffer(uint32_t len)
      : bufptr_(new (std::nothrow) char[len]), offset_(0), buflen_(len) {}
  ~Buffer() {
    if (nullptr != bufptr_) {
      delete[] bufptr_;
      bufptr_ = nullptr;
    }
    bufptr_ = nullptr;
  }
};

class LogBuffer {
 public:
  LogBuffer(uint32_t bufferSize);
  LogBuffer();
  ~LogBuffer() = default;
  bool Push(const std::string& logment);
  Buffer* SwapBuffer();

  void ClearOffset(Buffer* buf);

 private:
  std::mutex mtx_;
  Buffer* curPtr_;
  Buffer head_;
  Buffer tail_;
};

}  // namespace log

}  // namespace base

}  // namespace fver

#endif