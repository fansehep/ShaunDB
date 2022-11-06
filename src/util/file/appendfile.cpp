#include "src/util/file/appendfile.hpp"

#include <fcntl.h>
#include <fmt/format.h>

#include <cstdio>
#include <cstring>

#include "src/base/log/logging.hpp"

extern "C" {
#include <assert.h>
#include <fcntl.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
}

namespace fver {

namespace util {

namespace file {



AppendFile::~AppendFile() { Close(); }

void AppendFile::Append(char* data, const size_t data_size) {
  assert(nullptr != data);
  if ((buf_.buflen_ - buf_.offset_) > data_size) {
    std::memcpy(buf_.bufptr_ + buf_.offset_, data, data_size);
    buf_.offset_ += data_size;
    return;
  }
  //
  // 当前 buf 在刷入 data_size 之后的空间
  auto buf_remain_size = buf_.buflen_ - buf_.offset_;
  std::memcpy(buf_.bufptr_ + buf_.offset_, data, buf_remain_size);
  buf_.offset_ += buf_remain_size;
  FlushBuffer();
  // 将 完整的 buf 刷入之后, 还剩下的容量
  auto data_remain_size = data_size - buf_remain_size;
  // 剩余少于 kAppendFileBufferSize/4 则直接复制到buf 中
  // 否则直接朝文件写入
  if (data_remain_size < (kAppendFileBufferSize / 4)) {
    std::memcpy(buf_.bufptr_ + buf_.offset_, data + buf_remain_size,
                data_remain_size);
    buf_.offset_ += data_remain_size;
  } else {
    char* data_ptr = data + buf_remain_size;
    WriteDirect(data_ptr, data_remain_size);
  }
}

//
void AppendFile::Init(const std::string& path, const std::string& filename) {
  fullFilename_ = fmt::format("{}/{}", path, filename);
  // 所有人可读
  fd_ = ::open(fullFilename_.c_str(), O_RDWR | O_CREAT, 0644);
  if (fd_ < 0) {
    LOG_ERROR("can not open path: {} filename: {}", path, filename);
    return;
  }
  // 路径 path
  path_ = std::string_view(fullFilename_.data(), path.size());
  filename_ =
      std::string_view(fullFilename_.data() + path.size() + 1, filename.size());
}

void AppendFile::Sync() { ::fsync(fd_); }

void AppendFile::FlushBuffer() {
  WriteDirect(buf_.bufptr_, buf_.offset_);
  buf_.offset_ = 0;
}

// 直接向 fd 写入
void AppendFile::WriteDirect(const char* data, const size_t data_size) {
  assert(nullptr != data && -1 != fd_);
  auto write_size = ::write(fd_, data, data_size);
  assert(write_size == data_size);
}

const std::string_view& AppendFile::getPath() { return path_; }

const std::string_view& AppendFile::getFileName() { return filename_; }

uint32_t AppendFile::getFileSize() {
  struct ::stat stat_buf;
  ::fstat(fd_, &stat_buf);
  return stat_buf.st_size;
}

// 4 * 1024 = 4 kb
// 400kb
static thread_local char tread_buf[4096000] = {0};

void AppendFile::Read(std::string* data) {
  // seek 到文件开头, 再读取数据
  lseek(fd_, 0, SEEK_SET);
  while (true) {
    auto read_size = ::read(fd_, tread_buf, sizeof(tread_buf));
    if (read_size <= 0) {
      return;
    }
  //  LOG_INFO("read_size = {}", read_size);
    *data += std::string_view(tread_buf, read_size);
  }
}

void AppendFile::Close() {
  if (fd_ != -1) {
    FlushBuffer();
    ::sync();
    ::close(fd_);
    fd_ = -1;
  }
}

void AppendFile::Clear() {
  ftruncate(fd_, 0);
  lseek(fd_, 0, SEEK_SET);
}

}  // namespace file

}  // namespace util

}  // namespace fver