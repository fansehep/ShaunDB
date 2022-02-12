#include <thread>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <future>
#include <inttypes.h>

int sayhello(int a)
{
  std::cout << "sayhello : this_thread::get_id() = " << std::this_thread::get_id() << std::endl;
  return printf("hello,world!\n");
}


int main()
{
  std::cout << "this_thread::get_id() = " << std::this_thread::get_id() << std::endl;
  std::packaged_task<int(int)> mypt(sayhello);
  std::thread ta(std::ref(mypt), 1);
  ta.join();
  std::future<int> result = mypt.get_future();
  std::cout << result.get() << std::endl;
  return 0;
}