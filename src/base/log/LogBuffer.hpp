#ifndef SRC_BASE_LOG_LOGBUFFER_H_
#define SRC_BASE_LOG_LOGBUFFER_H_

#include <string.h>
#include "src/base/TimeStamp.hpp"
#include <ostream>
#include <string>

namespace fver::base::log {

class LogBuffer {
 public:
  LogBuffer(uint32_t maxsize);
  LogBuffer();
  ~LogBuffer();
  bool Push(const std::string& str);
  bool Push(const char* str, uint32_t len);
  bool Push(const char* str);
  bool Clear();
  bool Expansion(uint32_t bufsize);
  char* GetBufferPtr();
  uint32_t GetCurrentSize();
  uint32_t GetMaxSize();
  void SetBufHorSize(double bufsize);
  bool IsSync();
  bool IsChangeBuffer();
  static constexpr uint32_t kSmallBufferSize = 9192;
  static constexpr uint32_t kMidBufferSize = 16384;
  static constexpr uint32_t kBigBufferSize = 65546;

 private:
  uint32_t maxsize_;
  double bufhorisize_;
  char* bufferptr_;
  uint32_t cursize_;
};

}  // namespace fver::base::log

#endif