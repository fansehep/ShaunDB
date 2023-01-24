#include "src/util/file/readonlyfile.hpp"

#include <fcntl.h>
#include "fmt/format.h"
#include <sys/mman.h>
#include <sys/stat.h>

#include "src/base/log/logging.hpp"

namespace fver {

namespace util {

namespace file {

ReadonlyFile::ReadonlyFile() : mmap_base_ptr_(nullptr) {}

bool ReadonlyFile::Init(const std::string& path, const std::string& filename) {
  fullfilename_ = fmt::format("{}/{}", path, filename);
  auto fd = ::open(filename.c_str(), O_RDONLY);
  if (fd < 0) [[unlikely]] {
    LOG_ERROR("open file {} error_reasion: {}", fullfilename_,
              ::strerror(errno));
    return false;
  }
  struct ::stat file_stat;
  if (::stat(fullfilename_.c_str(), &file_stat) != 0) [[unlikely]] {
    LOG_WARN("stat file {} error!", fullfilename_);
    return false;
  }
  length_ = file_stat.st_size;
  if (length_ > 0) {
    mmap_base_ptr_ = static_cast<char*>(
        ::mmap(nullptr, length_, PROT_READ, MAP_SHARED, fd, 0));
    if (mmap_base_ptr_ == MAP_FAILED) {
      LOG_ERROR("mmap file {} error!", fullfilename_);
      return false;
    }
  }
  path_ = std::string_view(fullfilename_.data(), path.size());
  filename_ =
      std::string_view(fullfilename_.data() + path.size(), filename.size());
  ::close(fd);
  return true;
}

bool ReadonlyFile::Read(const uint64_t offset, const size_t n,
                        std::string_view* result) {
  assert(mmap_base_ptr_ != nullptr);
  if ((offset + n) > length_) {
    LOG_WARN("readonlyfile {} size error!", fullfilename_);
    return false;
  }
  *result = std::string_view(mmap_base_ptr_ + offset, n);
  return true;
}

size_t ReadonlyFile::getLength() { return length_; }

void ReadonlyFile::Close() {
  if (nullptr != mmap_base_ptr_) {
    ::munmap(static_cast<void*>(mmap_base_ptr_), length_);
  }
}

ReadonlyFile::~ReadonlyFile() { Close(); }

std::string ReadonlyFile::getPath() { return std::string(path_); }

std::string ReadonlyFile::getFileName() { return std::string(filename_); }

}  // namespace file
}  // namespace util
}  // namespace fver
