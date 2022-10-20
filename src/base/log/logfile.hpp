#ifndef SRC_BASE_LOG_LOGFILE_H_
#define SRC_BASE_LOG_LOGFILE_H_
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>

#include <string>

#include "src/base/noncopyable.hpp"
#include "src/base/timestamp.hpp"

using ::fver::base::NonCopyable;
using ::fver::base::TimeStamp;

namespace fver {
namespace base {
namespace log {

class LogFile : public NonCopyable {
 public:
  LogFile(const std::string& path);
  LogFile(const std::string& path, const std::string& logPrename);
  LogFile();
  ~LogFile();
  /**
   * path: the save log path.
   * @return: init success return true,
   *    otherwise return false.
   */
  bool SetLogPath(const std::string& path);
  
  bool SetLogPath(const std::string& path, const std::string& logPrename);
  void Write(const std::string& str);
  void WriteStr(const char* str, const int size);
  /**
   * the virtual filesystem will have a cache.
   * Sync can make the cache write to the disk. 
   */
  void Sync();
  void Change();
 private:
  std::FILE* fileptr_;
  std::string curLogPath_;
  std::string curLogName_;
  TimeStamp nowTime_;
};

}  // namespace log
}  // namespace base
}  // namespace fver

#endif