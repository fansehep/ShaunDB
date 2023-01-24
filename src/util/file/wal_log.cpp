#include "src/util/file/wal_log.hpp"

#include "fmt/format.h"

extern "C" {
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/fcntl.h>
}

#include <cstdio>

#include "src/base/log/logging.hpp"

namespace fver {

namespace util {

namespace file {

bool WalLog::Init(const std::string &path, const std::string &filename) {
  fd_ = -1;
  fullFilename_ = fmt::format("{}{}", path, filename);
  // O_RDWR 以可读可写权限打开文件
  // O_CREAT 如果文件不存在, 那么就创建
  // S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP : 打开文件权限位
  // S_IRUSR: 所有者拥有读权限
  // S_IWUSR: 所有者拥有写权限
  fd_ = ::open(fullFilename_.c_str(), O_RDWR | O_CREAT | O_SYNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
  if (fd_ < 0) {
    LOG_WARN("wallog open {} error!", fullFilename_);
    return false;
  }
  LOG_INFO("wal_log: {} init success", fullFilename_);
  path_ = std::string_view(fullFilename_.c_str(), path.size());
  fileName_ =
      std::string_view(fullFilename_.data() + path.size(), filename.size());
  return true;
}

void WalLog::AddRecord(const char *data, const size_t data_size) {
  assert(fd_ > 0);
  auto simple_write_size = write(fd_, data, data_size);
  assert(simple_write_size == data_size);

  // sync 操作, 保证数据落盘
  // but 根据 posix 文件系统语义, 当此操作发生断电 / 宕机行为
  // 是一个 ub 行为. >_<
  // 默认不 sync
  // ::fsync(fd_);
}

void WalLog::Close() {
  if (fd_ > 0) {
    ::close(fd_);
    fd_ = -1;
  }
}

WalLog::~WalLog() { Close(); }

uint32_t WalLog::getFileSize() {
  struct ::stat stat_buf;
  ::fstat(fd_, &stat_buf);
  return stat_buf.st_size;
}

void WalLog::SeekBegin() { ::lseek(fd_, 0, SEEK_SET); }

int WalLog::Read(char *buf_ptr, const size_t size) {
  SeekBegin();
  return ::read(fd_, buf_ptr, size);
}

}  // namespace file

}  // namespace util

}  // namespace fver