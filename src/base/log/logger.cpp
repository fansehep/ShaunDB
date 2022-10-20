#include "src/base/log/logger.hpp"

#include <memory>

#include "src/base/log/logthread.hpp"

namespace fver {
namespace base {
namespace log {

Logger::~Logger() {
#ifdef DEBUG
  fmt::print("logger disconstruct!\n");
#endif
}

Logger::Logger()
    : isSync_(false), buf_(4194304), sumWrites_(0), isHolderByThread_(true) {}

Logger::Logger(uint32_t bufsize)
    : isSync_(false), buf_(bufsize), sumWrites_(0), isHolderByThread_(true) {}

LogImp::LogImp(LogThread* thread) {
  log = std::make_shared<Logger>();
  thread->AddLogger(log);
}

LogImp::~LogImp() {
  log->isHolderByThread_ = false;
#ifdef FVER_LOG_DEBUG
  fmt::print("logimp disconstruct!\n");
#endif
}

}  // namespace log
}  // namespace base

}  // namespace fver