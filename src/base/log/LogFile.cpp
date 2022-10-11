#include "src/base/log/LogFile.hpp"

#include <fcntl.h>
#include <fmt/format.h>
#include <sys/stat.h>
#include <sys/types.h>

namespace fver::base::log {
LogFile::LogFile(const char* logpath) : logpath_(logpath), logfd_(-1) {
  timestamp_ = fver::base::TimeStamp::Now();
  curlogfilename_ = timestamp_.ToFormatToday() + ".log";
  auto tempfilepath = logpath_ + curlogfilename_;
  logfd_ = open(tempfilepath.c_str(), O_CREAT | O_RDWR);
  if (logfd_ < 0) {
    fmt::print("{} {} {} :error can not open file! path: {}",
               timestamp_.ToFormatTodayNowMs(), __FILE__, __LINE__, logpath);
  }
}

LogFile::LogFile(const std::string& logpath) : logpath_(logpath), logfd_(-1) {
  timestamp_ = fver::base::TimeStamp::Now();
  curlogfilename_ = timestamp_.ToFormatToday() + ".log";
  auto tempfilepath = logpath_ + curlogfilename_;
  logfd_ = open(tempfilepath.c_str(), O_CREAT | O_RDWR);
  if (logfd_ < 0) {
    fmt::print("{} {} {} :error can not open file! path: {}",
               timestamp_.ToFormatTodayNowMs(), __FILE__, __LINE__, logpath);
  }
}

LogFile::LogFile() {}

LogFile::~LogFile() {
  if (logfd_ > 0) {
    close(logfd_);
  }
}

bool LogFile::FasyncToDisk() {
  if (0 == fsync(logfd_)) {
    return true;
  }
  return false;
}

bool LogFile::Write(const char* str, int len) {
  auto write_len = write(logfd_, str, len);
  if (write_len == len) {
    return true;
  }
  return false;
}

bool LogFile::Write(const std::string& str) {
  auto write_len = write(logfd_, str.c_str(), str.size());
  if (write_len == static_cast<long int>(str.size())) {
    return true;
  }
  return false;
}

bool LogFile::Clear() {
  ftruncate(logfd_, 0);
  lseek(logfd_, 0, SEEK_SET);
  return true;
}

bool LogFile::ChangeLogFile() {
  timestamp_ = fver::base::TimeStamp::Now();
  curlogfilename_ = timestamp_.ToFormatToday() + ".log";
  auto tempfilepath = logpath_ + curlogfilename_;
  logfd_ = open(tempfilepath.c_str(), O_CREAT | O_RDWR);
  if (logfd_ < 0) {
    fmt::print("{} {} {} :error can not open file! path: {}",
               timestamp_.ToFormatTodayNowMs(), __FILE__, __LINE__, logpath_);
    return false;
  }
  return true;
}
std::string LogFile::GetCurrentLogFileName() { return this->curlogfilename_; }

std::string LogFile::GetCurrentLogPathName() { return this->logpath_; }

void LogFile::SetLogPath(const std::string& logpath) {
  logpath_ = logpath;
  timestamp_ = fver::base::TimeStamp::Now();
  curlogfilename_ = timestamp_.ToFormatToday() + ".log";
  auto tempfilepath = logpath_ + curlogfilename_;
  logfd_ = open(tempfilepath.c_str(), O_CREAT | O_RDWR);
  if (logfd_ < 0) {
    fmt::print("{} {} {} :error can not open file! path: {}",
               timestamp_.ToFormatTodayNowMs(), __FILE__, __LINE__, logpath);
  }
}

}  // namespace fver::base::log