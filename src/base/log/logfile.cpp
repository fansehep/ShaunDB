#include "src/base/log/logfile.hpp"

#include "fmt/format.h"

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
#ifdef FVER_LOG_DEBUG
    fmt::print("log file {} {} close\n", curLogPath_, curLogName_);
#endif
  }
  fileptr_ = nullptr;
}

LogFile::LogFile(const std::string& path) : fileptr_(nullptr) {
  nowTime_ = TimeStamp::Now();
  assert(path.back() != '/');
  curLogName_ = fmt::format("{}/{}.log", path, nowTime_.ToFormatToday());
  fileptr_ = std::fopen(curLogName_.c_str(), "wbx");
#ifdef FVER_LOG_DEBUG
  if (nullptr == fileptr_) {
    fmt::print("{} {} {} logfile init: {} has current log.\n",
               nowTime_.ToFormatTodayNowMs(), __FILE__, __LINE__, path);
  }
#endif
  fileptr_ = std::fopen(curLogName_.c_str(), "w+");
  assert(fileptr_ != nullptr);
  // init
  curLogPath_ = path;
}

LogFile::LogFile(const std::string& path, const std::string& logPrename) {
  nowTime_ = TimeStamp::Now();
  assert(path.back() != '/');
  curLogName_ = fmt::format("{}/{}-{}-{}.log", path, logPrename, rand(),
                            nowTime_.ToFormatToday());
  fileptr_ = std::fopen(curLogName_.c_str(), "wbx");
#ifdef FVER_LOG_DEBUG
  if (nullptr == fileptr_) {
    fmt::print("{} {} {} logfile init: {} has current log.\n",
               nowTime_.ToFormatTodayNowMs(), __FILE__, __LINE__, path);
  }
#endif
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
#ifdef FVER_LOG_DEBUG
  if (nullptr == fileptr_) {
    fmt::print("{} {} {} logfile init: {} has current log.",
               nowTime_.ToFormatTodayNowMs(), __FILE__, __LINE__, path);
  }
#endif
  fileptr_ = std::fopen(curLogName_.c_str(), "w+");
  assert(fileptr_ != nullptr);
  // init
  if (fileptr_ == nullptr) {
    return false;
  }
  curLogPath_ = path;
  return true;
}

bool LogFile::SetLogPath(const std::string& path,
                         const std::string& logPrename) {
  nowTime_ = TimeStamp::Now();
  //  assert(path.back() != '/');
  curLogName_ =
      fmt::format("{}/{}-{}.log", path, logPrename, nowTime_.ToFormatLogName());
  fileptr_ = std::fopen(curLogName_.c_str(), "wbx");
#ifdef FVER_LOG_DEBUG
  if (nullptr == fileptr_) {
    fmt::print("{} {} {} logfile init: {} has current log.",
               nowTime_.ToFormatTodayNowMs(), __FILE__, __LINE__, path);
  }
#endif
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
#ifdef DEBUG
  assert(fileptr_ != nullptr);
  auto write_size =
      std::fwrite(logment.data(), sizeof(char), logment.size(), fileptr_);
  assert(write_size == logment.size());
#else
  std::fwrite(logment.c_str(), sizeof(char), logment.size(), fileptr_);
#endif
}

void LogFile::WriteStr(const char* str, const int size) {
#ifdef DEBUG
  assert(fileptr_ != nullptr);
  assert(std::fwrite(str, sizeof(char), size, fileptr_), size);
#else
  std::fwrite(str, sizeof(char), size, fileptr_);
#endif
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