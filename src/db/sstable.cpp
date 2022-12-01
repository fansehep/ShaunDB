#include "src/db/sstable.hpp"

#include <fmt/format.h>
#include <sys/mman.h>

#include <cstring>

#include "src/base/log/logging.hpp"

extern "C" {
#include <error.h>
#include <fcntl.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
}

namespace fver {

namespace db {

SSTable::SSTable() : isMmap_(false), mmapBasePtr_(nullptr), isOpen_(false) {}

SSTable::~SSTable() {
  if (isOpen_ == true) {
    ::close(fd_);
  }
  if (true == isMmap_) {
    CloseMmap();
  }
}

int SSTable::getFd() { return fd_; }

bool SSTable::Init(const std::string& path, const std::string& filename,
                   const uint32_t level, const uint32_t number) {
  fullfilename_ = fmt::format("{}/{}", path, filename);
  fd_ = ::open(fullfilename_.c_str(), O_RDWR | O_CREAT, 0655);
  if (fd_ < 0) {
    LOG_WARN("sstable path: {} name: {} open error: {}", path, filename,
             ::strerror(errno));
    return false;
  }
  fileName_ = filename;
  filePath_ = path;
  sstable_number_ = number;
  sstable_level_ = level;
  isOpen_ = true;
  return true;
}

void SSTable::Close() { ::close(fd_); }

auto SSTable::getLevel() { return sstable_level_; }

void SSTable::setLevel(const uint32_t n) { sstable_level_ = n; }

void SSTable::setNumber(const uint32_t n) { sstable_number_ = n; }

//
uint32_t SSTable::getNumber() { return sstable_number_; }

uint32_t SSTable::getFileSize() {
  struct ::stat file_stat;
  if (-1 == ::fstat(fd_, &file_stat)) {
    LOG_ERROR("get file: {} error: {}", fullfilename_, ::strerror(errno));
    assert(false);
  }
  return file_stat.st_size;
}

bool SSTable::InitMmap() {
  struct ::stat stat_buf;
  if (::stat(fullfilename_.c_str(), &stat_buf) != 0) [[unlikely]] {
    LOG_ERROR("mmap path: {} error: {}", fullfilename_, ::strerror(errno));
    return false;
  }
  fileLength_ = stat_buf.st_size;
  mmapBasePtr_ = static_cast<char*>(
      ::mmap(nullptr, fileLength_, PROT_READ, MAP_SHARED, fd_, 0));
  if (mmapBasePtr_ == MAP_FAILED) {
    LOG_ERROR("mmap file: {} error: {}", fullfilename_, ::strerror(errno));
    return false;
  }
  ::close(fd_);
  isOpen_ = false;
  isMmap_ = true;
  return true;
}

void SSTable::CloseMmap() {
  ::munmap(static_cast<void*>(mmapBasePtr_), fileLength_);
}

char* SSTable::getMmapPtr() { return mmapBasePtr_; }

}  // namespace db

}  // namespace fver