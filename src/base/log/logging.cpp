#include "src/base/log/logging.hpp"

#include "src/base/log/logger.hpp"
#include "src/base/log/logthread.hpp"

fver::base::log::LogThread logthread;

thread_local fver::base::log::LogImp logimp(&logthread);

namespace fver {

namespace base {

namespace log {


//FIXME, in GTEST can not write to file
void Init(const std::string& logpath, const int lev, const bool isSync,
          const std::string& logPrename, const uint32_t buf_size) {
  logthread.Init(logpath, lev, isSync, logPrename, buf_size);
}

}  // namespace log

}  // namespace base

}  // namespace fver
