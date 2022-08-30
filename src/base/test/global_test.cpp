#include "global_test.hpp"

#include <iostream>
#include <thread>
#include <vector>
int main() {
  gvfl::base::test a;
  std::vector<std::thread> workers;
  for (int i = 0; i < 5; i++) {
    workers.emplace_back([&, i]() {
      gvfl::base::test::b = i;
      std::cout << gvfl::base::test::b << std::endl;
    });
  }
  for (auto& iter : workers) {
    iter.join();
  }
  printf("hello, i am main!\n");
  return 0;
}