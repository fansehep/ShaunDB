#include "../Thread.h"
#include <vector>
#include "../CurrentThread.h"
#include <stdio.h>

//* g++/clang++ Thread_test.cpp ../Thread.cpp ../Logging.cpp ../LogStream.cpp ../Exception.cpp -o Thread_test -lpthread -std=c++17 -lboost_thread
int main()
{
  printf("main pid = %d, tid = %d \n", ::getpid(), UBERS::CurrentThread::tid());

  //*



  return 0;
}