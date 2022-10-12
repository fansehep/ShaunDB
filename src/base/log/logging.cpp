#include "src/base/log/logging.hpp"

#include "src/base/log/logger.hpp"
#include "src/base/log/logthread.hpp"

fver::base::log::LogThread logthread;

thread_local fver::base::log::Logger logger(&logthread);

namespace fver {

namespace base {

namespace log {

void Init(const std::string logpath, const int lev) {
  logthread.Init(logpath, lev);
}

}  // namespace log

}  // namespace base

}  // namespace fver