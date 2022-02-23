#include "../AsyncLogThread.h"
#include "../Thread.h"
#include "../File.h"
#include "../Logging.h"
#include "../TimeStamp.h"
#include "../ThreadPool.h"
#include "../LogStream.h"
#include "../LogFile.h"
#include "../Exception.h"
#include "../TimeStamp.h"
#include "../CurrentThread.h"
#include <unistd.h>
#include <fcntl.h>

UBERS::base::AsyncLogThread* CurrentLog = nullptr;

//* 正常使用的情况下，需要重定向日志到 AsyncLogThread 
void AsyncLogOutToFile(const char* msg, size_t len)
{
  CurrentLog->append(msg, len);
}

void OneThreadLog()
{
  LOG_INFO << "such as a ";
}

void ThreadPoolLogTest()
{
  UBERS::base::ThreadPool pool;
  printf("CurrentThread Id = %d", UBERS::CurrentThread::tid());
  pool.Start(8);
  for(int i = 0; i < 100; ++i)
  {
      pool.AddTask(std::bind(OneThreadLog));
  }

}
int main(void)
{
  //* 传入日志名称
  UBERS::base::AsyncLogThread log(::basename("log"));
  log.start();
  CurrentLog = &log;
  UBERS::Logger::setOutPut(AsyncLogOutToFile);
  LOG_INFO << " this is a test";
  ThreadPoolLogTest();
 // sleep(4);
  printf("000xxxx000\n");
  log.stop();
  return 0;
}
