#include <cstdio>
#。/include <mutex>
#include <thread>
#include <atomic>

class Solution
{
public:
  void start()
  {
    std::thread thread_1(&Solution::add, this, 20000000);
    std::thread thread_2(&Solution::add, this, 20000000);
    thread_1.join();
    thread_2.join();
    printf("the sum = %d", sum.load());
  }
private:
  void add(int n)
  {
    for(int i = 0; i < n; ++i)
    {
      sum.fetch_add(1);
    }
  }
  std::atomic<int> sum;
};

int main(void)
{
  Solution().start();
  return 0;
}