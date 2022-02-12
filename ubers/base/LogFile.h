#ifndef _UBERS_LOGFILE_H_
#define _UBERS_LOGFILE_H_

#include <memory>
#include <mutex>
#include <string_view>
#include <boost/noncopyable.hpp>

namespace UBERS
{
class WriteFile;
class LogFile : public boost::noncopyable
{
public:
  explicit LogFile(std::string_view basename, size_t rollsize = 1024 * 64, bool threadsafe = true);
  ~LogFile() = default;
  void append(const char* logline, int len);
  void flush();
  void RollFile();

private:
  void GetLogFileName(std::string& name);
  void append_unlocked(const char* log, size_t len);
  const std::string BaseName_;
  int count_ = 0;
  size_t RollSize_;
  std::unique_ptr<std::mutex> mutex_;
  std::unique_ptr<WriteFile> file_;

  const static int kFlushInterval_ = 5;
};
}
#endif