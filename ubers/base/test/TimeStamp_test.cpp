#include <iostream>
#include <ubers/base/TimeStamp.h>
using namespace UBERS;

int main()
{
  TimeStamp nowTime(TimeStamp::now());
  std::cout << nowTime.ToFormattedString(true) << std::endl;
  return 0;
}
