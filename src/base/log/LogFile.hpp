#ifndef SRC_BASE_LOG_LOGFILE_H_
#define SRC_BASE_LOG_LOGFILE_H_

#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>
#include <string>
#include "../NonCopyable.hpp"
#include "../TimeStamp.hpp"
namespace fver::base::log {

class LogFile : public NonCopyable {
 public:
  LogFile(const char* logpath);
  LogFile(const std::string& logpath);
  LogFile();
  ~LogFile();
  void SetLogPath(const std::string& logpath);
  bool Write(const char* str, int len);
  bool Write(const std::string& str);
  // clear current log file all info
  bool Clear();
  bool FasyncToDisk();
  // change next logfile
  // default change time is 24h
  bool ChangeLogFile();
  std::string GetCurrentLogFileName();
  std::string GetCurrentLogPathName();
 private:
  fver::base::TimeStamp timestamp_;
  std::string curlogfilename_;
  std::string logpath_;
  int logfd_;
};
}  // namespace fver::base::log
#endif