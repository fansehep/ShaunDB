#ifndef SRC_BASE_LOG_THREADLOCALBUFFER_H_
#define SRC_BASE_LOG_THREADLOCALBUFFER_H_
#include <fmt/format.h>
#include <stdio.h>

#include <mutex>
#include <string>
#include <string_view>
#include <atomic>

#include "../NonCopyable.hpp"
#include "LogBuffer.hpp"

namespace fver::base::log {


class ThreadLocalBuffer : public NonCopyable {
 public:
  ThreadLocalBuffer(uint32_t bufsize);
  ThreadLocalBuffer();
  ~ThreadLocalBuffer() = default;
  void Push(const std::string& buf);
  void Push(const char* buf);
  void Push(char* buf, int len);
  void ChangeBufPtr();
  bool IsChangeBufPtr();
  bool IsFillThresold();
  void AddUnlogTimes();
  void ClearUnlogTimes();
  int GetUnlogTimes();
  void SetBufhorSize(double hor);
  double GetBufhorSize();
  char* GetBeginPtr();
  void ClearTmpBuf();
  uint32_t GetSize();
  char* GetCurBufPtr() {return prebuf_.GetBufferPtr();}
  int GetCurBufSize() {return prebuf_.GetCurrentSize();}
 private:
  double bufhorsize_;
  std::atomic<int> unlogtimes_;
  std::mutex mtx_;
  LogBuffer prebuf_;
  LogBuffer tailbuf_;
  LogBuffer* curbufptr_;
  LogBuffer* tmpbufptr_;
};

}  // namespace fver::base::log
#endif