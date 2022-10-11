#include "src/base/log/Logging.hpp"
#include "src/base/log/AsyncLoggingThread.hpp"


fver::base::log::AsyncLogThread* glogthread = nullptr;

thread_local fver::base::log::LoggerImp logger;