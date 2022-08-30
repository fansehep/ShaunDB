#ifndef GLOBAL_TEST_HPP_
#define GLOBAL_TEST_HPP_

#include <stdio.h>

#include <thread>

namespace gvfl::base {

class test {
 public:
  test() { printf("test constructor!\n"); }
  ~test() { printf("test distructor!\n"); }
  void Print(int t_b) { a = t_b; }
  thread_local static int b;

 private:
  int a;
};

}  // namespace gvfl::base
#endif