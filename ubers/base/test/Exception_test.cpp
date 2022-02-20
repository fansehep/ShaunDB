#include "../Exception.h"
#include <stdio.h>
class Error_m
{
public:
  Error_m() = default;
  void test()
  {
    ThrowError();
  }
private:
  void ThrowError()
  {
    int a = 1;
    throw UBERS::Exception("i like coding!");
  }
};
int main(void)
{
  try
  {
    Error_m().test();
  }
  catch (const UBERS::Exception& ex)
  {
    printf("%s", ex.what());
    printf("%s", ex.stackTrace());
  }
  return 0;
}