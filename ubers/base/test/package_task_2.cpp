#include <thread>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <future>

int sayhello(int a)
{
  std::cout << "hello, world!\n" << std::endl;
  return 2;
}


int main()
{
  std::packaged_task<int(int)> my_function(sayhello);
  my_function(1);




  return 0;
}