#include <iostream>
#include "../TimeStamp.h"
using namespace UBERS;

int main()
{
  TimeStamp nowTime(TimeStamp::now());
  std::cout << nowTime.ToFormattedString(false) << std::endl;
  return 0;
}
