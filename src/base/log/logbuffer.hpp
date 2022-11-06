#ifndef SRC_BASE_LOG_LOGBUFFER_H_
#define SRC_BASE_LOG_LOGBUFFER_H_

#include <cstring>
#include <condition_variable>
#include <cstring>
#include <memory>
#include <new>
#include <vector>
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
      : bufptr_(new (std::nothrow) char[len]), offset_(0), buflen_(len) {
    std::memset(bufptr_, 0, len);
  }
  ~Buffer() {
    if (nullptr != bufptr_) {
      delete[] bufptr_;
      bufptr_ = nullptr;
    }
    bufptr_ = nullptr;
  }
};


class Logger;

// for libfmt;
class VecLogBuffer {
 public:
  friend Logger;
  VecLogBuffer(uint32_t buffer_size);
  VecLogBuffer();
  ~VecLogBuffer() = default;
  std::vector<char>* SwapBuffer();
  std::mutex* getMutex();

 private:
  
  std::mutex mtx_;
  std::vector<char>* curPtr_;
  std::vector<char> head_;
  std::vector<char> tail_;
};

}  // namespace log

}  // namespace base

}  // namespace fver

#endif