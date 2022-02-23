#include "../Logging.h"
#include "../LogStream.h"
#include "../LogFile.h"

//g++ Log_test.cpp ../LogStream.cpp ../Logging.cpp ../File.cpp ../Thread.cpp ../Exception.cpp ../LogFile.cpp ../TimeStamp.cpp -o log_test -lpthread -lboost_thread
int main(void)
{
  /*
  * 默认输出到标准输出上
  *
  */
  LOG_DEBUG << "debug";
  LOG_ERROR << "error";
//  LOG_FATAL << "FATAL";
  LOG_INFO << "INFO";
//  LOG_OFF << "off";
  LOG_SYSERR << "syserr";
  LOG_WARN << "warn";


  return 0;
}