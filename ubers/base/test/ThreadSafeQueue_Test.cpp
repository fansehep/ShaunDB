#include "../ThreadSafeQueue.hpp"

int main(void)
{
  UBERS::base::ThreadSafeQueue<int> a;
  auto x = a.empty();
  a.Push(1);

  return 0;
}