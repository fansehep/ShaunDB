#ifndef SRC_UTIL_FILE_READONLY_H_
#define SRC_UTIL_FILE_READONLY_H_

#include <cstdio>
#include <cstring>
#include <string>
#include <string_view>
#include "fmt/format.h"

#include "src/base/noncopyable.hpp"
#include <fcntl.h>


using ::fver::base::NonCopyable;

namespace fver {

namespace util {

namespace file {

// 使用 mmap 来快速读取的只读文件
class ReadonlyFile : public NonCopyable {
public:
  ReadonlyFile();
  bool Init(const std::string& path, const std::string& filename);
  void Close();
  ~ReadonlyFile();
  bool Read(const uint64_t offset, const size_t n, std::string_view* result);
  size_t getLength();
  std::string getPath();
  std::string getFileName();
private:
  char* mmap_base_ptr_;
  size_t length_;
  std::string fullfilename_;
  std::string_view path_;
  std::string_view filename_;
};

}  // namespace file
}  // namespace util
}  // namespace fver

#endif