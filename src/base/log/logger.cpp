#include "src/base/log/logger.hpp"

#include "src/base/log/logthread.hpp"

namespace fver {
namespace base {
namespace log {

Logger::Logger() : isSync_(false), buf_(1024), sumWrites_(0) {}

Logger::Logger(uint32_t bufsize)
    : isSync_(false), buf_(bufsize), sumWrites_(0) {}

Logger::Logger(LogThread* logthread, uint32_t bufsize)
    : isSync_(false), buf_(bufsize), sumWrites_(0) {
  logthread->AddLogger(this);
}

}  // namespace log
}  // namespace base

}  // namespace fver