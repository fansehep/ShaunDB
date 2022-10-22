#ifndef SRC_UTIL_FILE_WAL_LOG_H_
#define SRC_UTIL_FILE_WAL_LOG_H_

#include <cstdio>

#include "src/base/noncopyable.hpp"
#include <string>
#include <string_view>

extern "C" {
#include <fcntl.h>
}

using ::fver::base::NonCopyable;

namespace fver {

namespace util {

namespace file {

// 预写日志
class WalLog : public NonCopyable {
 public:
  WalLog() = default;
  ~WalLog();
  bool Init(const std::string& path, const std::string& filename);
  void AddRecord(const char* data, const size_t data_size);
  void Close();
 private:
  std::FILE* fileptr_;
  std::string fullFilename_;
  std::string_view fileName_;
  std::string_view path_;
};

}  // namespace file

}  // namespace util

}  // namespace fver

#endif