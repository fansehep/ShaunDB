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
class AppendFile {
 public:
  AppendFile() : buf_(kAppendFileBufferSize), fd_(-1) {}
  ~AppendFile();

  /*
    // 初始化
  */
  void Init(const std::string& path, const std::string& filename);
  void Close();

  void Append(char* data, const size_t data_size);

  // 从当前 sstabel 中的文件中读取数据
  void Read(std::string* data);

  // 清空当前文件内容
  void Clear();

  /*
     将文件数据中的所有数据全部刷入到磁盘中,
     但并不会将buf 的数据刷入到磁盘中.
  */
  void Sync();

  const std::string_view& getPath();
  /*

  */
  const std::string_view& getFileName();
  //
  void FlushBuffer();

  // 获取当前文件的容量
  uint32_t getFileSize();

  int getFd() const {
    return fd_;
  }

 private:
  //
  void WriteDirect(const char* data, const size_t data_size);
  //
  base::log::Buffer buf_;

  // 完整的 full_name
  std::string fullFilename_;
  //
  std::string_view path_;
  //
  std::string_view filename_;
  int fd_;
};

}  // namespace file
}  // namespace util

}  // namespace fver

#endif