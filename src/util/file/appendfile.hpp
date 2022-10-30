#ifndef SRC_UTIL_FILE_APPENDFILE_H_
#define SRC_UTIL_FILE_APPENDFILE_H_

#include <cstdio>

#include "src/base/log/logbuffer.hpp"
#include "src/base/noncopyable.hpp"

using ::fver::base::NonCopyable;
using ::fver::base::log::Buffer;

namespace fver {
namespace util {

namespace file {

static constexpr int kAppendFileBufferSize = 65536;

// not threadsafe
// 带有缓存的文件类实现.
class AppendFile : public NonCopyable {
 public:
  AppendFile() : buf_(kAppendFileBufferSize), fileptr_(nullptr) {}
  ~AppendFile();
  void Init(const std::string& path, const std::string& filename);
  void Append(char* data, const size_t data_size);
  void Sync();
  std::string getPath();
  std::string getFileName();
 private:
  void FlushBuffer();
  void WriteDirect(const char* data, const size_t data_size);
  base::log::Buffer buf_;
  std::string fullFilename_;
  std::string_view path_;
  std::string_view filename_;
  std::FILE* fileptr_;
};

}  // namespace file
}  // namespace util

}  // namespace fver

#endif