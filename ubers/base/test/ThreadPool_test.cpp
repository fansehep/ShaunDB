#define NDEBUG
#include "../ThreadPool.h"
#include "../Logging.h"
#include "../CurrentThread.h"
#include <unistd.h>
#include <stdio.h>
#include <functional>
#include <string>

//g++ ThreadPool_test.cpp ../Thread.cpp ../ThreadPool.cpp ../CurrentThread.h ../LogFile.cpp ../Logging.cpp ../LogStream.cpp ../Exception.cpp ../TimeStamp.cpp ../File.cpp -o ThreadPool ../ThreadSafeQueue.hpp  -lboost_thread -pthread -std=c++17

void PrintCurrentThreadInfo()
{
  printf("CurrentThread: %d\n", UBERS::CurrentThread::tid());
}

void PrintString(const std::string& str)
{
  LOG_INFO << str;
  //usleep(100 * 10);
}
//FIXME
inline void Test(int ThreadPoolSize)
{
  printf("unsigned threadnum = %d \n", ThreadPoolSize);
  LOG_WARN << "Test ThreadPool Size = " << ThreadPoolSize;
  UBERS::base::ThreadPool pool_; 
  pool_.SetThreadInitCallback(std::bind(PrintCurrentThreadInfo));
  pool_.Start(ThreadPoolSize);

  LOG_WARN << "Adding";
  pool_.AddTask(std::bind(PrintCurrentThreadInfo));
  pool_.AddTask(std::bind(PrintCurrentThreadInfo));

  for(int i = 0; i < 100; ++i)
  {
    char buf[32];
    snprintf(buf, sizeof(buf), "task %d", i);
    pool_.AddTask(std::bind(PrintString, (buf)));
  }

  LOG_WARN << "Down";
}

int main(void)
{
  Test(1);
  Test(2);
  Test(3);

  return 0;
}