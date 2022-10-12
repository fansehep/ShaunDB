#include "src/base/log/logfile.hpp"

#include <fmt/format.h>

#include <cassert>
#include <cstdio>

namespace fver {

namespace base {

namespace log {

LogFile::LogFile() : fileptr_(nullptr) {}

LogFile::~LogFile() {
  if (fileptr_ != nullptr) {
    Sync();
    std::fclose(fileptr_);
  }
  fileptr_ = nullptr;
}

LogFile::LogFile(const std::string& path) : fileptr_(nullptr) {
  nowTime_ = TimeStamp::Now();
  assert(path.back() != '/');
  curLogName_ = fmt::format("{}/{}.log", path, nowTime_.ToFormatToday());
  fileptr_ = std::fopen(curLogName_.c_str(), "wbx");
  if (nullptr == fileptr_) {
    fmt::print("{} {} {} logfile init: {} has current log.\n",
               nowTime_.ToFormatTodayNowMs(), __FILE__, __LINE__, path);
  }
  fileptr_ = std::fopen(curLogName_.c_str(), "w+");
  assert(fileptr_ != nullptr);
  // init
  curLogPath_ = path;
}

bool LogFile::SetLogPath(const std::string& path) {
  nowTime_ = TimeStamp::Now();
  assert(path.back() != '/');
  curLogName_ = fmt::format("{}/{}.log", path, nowTime_.ToFormatToday());
  fileptr_ = std::fopen(curLogName_.c_str(), "wbx");
  if (nullptr == fileptr_) {
    fmt::print("{} {} {} logfile init: {} has current log.",
               nowTime_.ToFormatTodayNowMs(), __FILE__, __LINE__, path);
  }
  fileptr_ = std::fopen(curLogName_.c_str(), "w+");
  assert(fileptr_ != nullptr);
  // init
  if (fileptr_ == nullptr) {
    return false;
  }
  curLogPath_ = path;
  return true;
}

void LogFile::Write(const std::string& logment) {
  assert(fileptr_ != nullptr);
  auto write_size =
      std::fwrite(logment.data(), sizeof(logment[0]), logment.size(), fileptr_);
  assert(write_size == logment.size());
}

void LogFile::WriteStr(const char* str, const int size) {
  assert(fileptr_ != nullptr);
  auto write_size = std::fwrite(str, sizeof(char), size, fileptr_);
  assert(write_size == size);
}

void LogFile::Sync() { std::fflush(fileptr_); }

void LogFile::Change() {
  assert(fileptr_ != nullptr);
  std::fclose(fileptr_);
  SetLogPath(curLogPath_);
}
}  // namespace log

}  // namespace base

}  // namespace fver