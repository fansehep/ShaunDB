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

// 单个线程 2 个 buffer, 所以一个线程的占用 4 + 8 = 12 MB 左右.
Logger::Logger()
    : isSync_(false), sumWrites_(0), buf_(0), isHolderByThread_(true) {}

Logger::Logger(uint32_t bufsize)
    : isSync_(false), sumWrites_(0), buf_(bufsize), isHolderByThread_(true) {}

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