#ifndef GLOBAL_TEST_HPP_
#define GLOBAL_TEST_HPP_

#include <stdio.h>

namespace gvfl::base {

class test {
 public:
  test() { printf("test constructor!\n"); }
  ~test() { printf("test distructor!\n"); }

 private:
};

}  // namespace gvfl::base
gvfl::base::test a;

#endif