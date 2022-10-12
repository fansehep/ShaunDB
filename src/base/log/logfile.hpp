#ifndef SRC_BASE_LOG_LOGFILE_H_
#define SRC_BASE_LOG_LOGFILE_H_
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>

#include <string>

#include "src/base/NonCopyable.hpp"
#include "src/base/TimeStamp.hpp"

using ::fver::base::NonCopyable;
using ::fver::base::TimeStamp;

namespace fver {
namespace base {
namespace log {

class LogFile : public NonCopyable {
 public:
  LogFile(const std::string& path);
  LogFile();
  ~LogFile();
  /**
   * path: the save log path.
   * @return: init success return true,
   *    otherwise return false.
   */
  bool SetLogPath(const std::string& path);
  
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