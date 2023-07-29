#pragma once

#include "src/log/logger.h"

namespace shaun {

static thread_local LoggerImpl logimp;

} // namespace shaun

#define Info(str, ...)                                                         \
  shaun::logimp.get_logger()->to_log(                                          \
      __FILE_NAME__, __LINE__, shaun::LogLevel::Info, str, ##__VA_ARGS__)

#define Warn(str, ...)                                                         \
  shaun::logimp.get_logger()->to_log(                                          \
      __FILE_NAME__, __LINE__, shaun::LogLevel::Warn, str, ##__VA_ARGS__)

#define Debug(str, ...)                                                        \
  shaun::logimp.get_logger()->to_log(                                          \
      __FILE_NAME__, __LINE__, shaun::LogLevel::Debug, str, ##__VA_ARGS__)

#define Trace(str, ...)                                                        \
  shaun::logimp.get_logger()->to_log(                                          \
      __FILE_NAME__, __LINE__, shaun::LogLevel::Trace, str, ##__VA_ARGS__)

#define Error(str, ...)                                                        \
  shaun::logimp.get_logger()->to_log(                                          \
      __FILE_NAME__, __LINE__, shaun::LogLevel::Error, str, ##__VA_ARGS__)

#define Exit(str, ...)                                                         \
  shaun::logimp.get_logger()->to_log(                                          \
      __FILE_NAME__, __LINE__, shaun::LogLevel::Exit, str, ##__VA_ARGS__)

#ifdef DEBUG

#define Dinfo(str, ...)                                                        \
  shaun::logimp.get_logger()->to_log(                                          \
      __FILE_NAME__, __LINE__, shaun::LogLevel::DInfo, str, ##__VA_ARGS__)

#define Dwarn(str, ...)                                                        \
  shaun::logimp.get_logger()->to_log(                                          \
      __FILE_NAME__, __LINE__, shaun::LogLevel::Dwarn, str, ##__VA_ARGS__)

#define Dtrace(str, ...)                                                       \
  shaun::logimp.get_logger()->to_log(                                          \
      __FILE_NAME__, __LINE__, shaun::LogLevel::Dtrace, str, ##__VA_ARGS__)
#else

#define Dinfo(str, ...) void(str);
#define Dwarn(str, ...) void(str);
#define Dtrace(str, ...) void(str);

#endif
