#ifndef GLOBAL_TEST_HPP_
#define GLOBAL_TEST_HPP_

#include <stdio.h>

namespace gvfl::base {

class test {
 public:
  test() { printf("test constructor!\n"); }
  ~test() { printf("test distructor!\n"); }
  void Print(int b) { a = b; }

 private:
  int a;
};

}  // namespace gvfl::base
static thread_local gvfl::base::test test_t;
#endif