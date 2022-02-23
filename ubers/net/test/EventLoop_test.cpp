#include "../EventLoop.h"
#include "../Channel.h"
#include "../Epoll.h"

using namespace UBERS;
using namespace UBERS::net;

void ThreadFunc()
{
  printf("CurrentThread Id = %d", CurrentThread::tid());
  EventLoop loop_2;
}

int main(void)
{
  printf("CurrentThread Id = %d", CurrentThread::tid());
  EventLoop loop_1;

  Thread thd(ThreadFunc);
  thd.start();

  return 0;
}