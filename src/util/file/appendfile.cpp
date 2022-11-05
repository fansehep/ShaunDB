#include "src/util/file/appendfile.hpp"

#include <fmt/format.h>

#include <cstdio>
#include <cstring>

extern "C" {
#include <assert.h>
}

namespace fver {

namespace util {

namespace file {

AppendFile::~AppendFile() {
  if (nullptr != fileptr_) {
    FlushBuffer();
    Sync();
    std::fclose(fileptr_);
  }
  fileptr_ = nullptr;
}

void AppendFile::Append(char* data, const size_t data_size) {
  assert(nullptr != data);
  if ((buf_.buflen_ - buf_.offset_) < data_size) {
    std::memcpy(buf_.bufptr_ + buf_.offset_, data, data_size);
    buf_.offset_ += data_size;
    return;
  }
  // 当前 buf 剩余的空间
  auto buf_remain_size = buf_.buflen_ - buf_.offset_;
  std::memcpy(buf_.bufptr_ + buf_.offset_, data, buf_remain_size);
  FlushBuffer();
  auto data_remain_size = data_size - buf_remain_size;
  // 剩余少于 kAppendFileBufferSize/4 则直接复制到buf 中
  // 否则直接朝文件写入
  if (data_remain_size < kAppendFileBufferSize / 4) {
    std::memcpy(buf_.bufptr_ + buf_.offset_, data + buf_remain_size,
                data_remain_size);
    buf_.offset_ += data_remain_size;
  } else {
    char* dataptr = data + buf_remain_size;
    WriteDirect(dataptr, data_remain_size);
  }
}

void AppendFile::Init(const std::string& path, const std::string& filename) {
  fullFilename_ = fmt::format("{}/{}", path, filename);
  fileptr_ = std::fopen(fullFilename_.c_str(), "w+");
  assert(nullptr != fileptr_);
  path_ = std::string_view(fullFilename_.data(), path.size());
  filename_ =
      std::string_view(fullFilename_.data() + path.size() + 1, filename.size());
}

void AppendFile::Sync() { std::fflush(fileptr_); }

void AppendFile::FlushBuffer() {
  WriteDirect(buf_.bufptr_, buf_.offset_);
  buf_.offset_ = 0;
}

// 直接向 fd 写入
void AppendFile::WriteDirect(const char* data, const size_t data_size) {
  assert(nullptr != data && nullptr != fileptr_);
  auto write_size = std::fwrite(data, sizeof(char), data_size, fileptr_);
  assert(write_size == data_size);
}

const std::string_view& AppendFile::getPath() { return path_; }

const std::string_view& AppendFile::getFileName() { return filename_; }

}  // namespace file

}  // namespace util

}  // namespace fver