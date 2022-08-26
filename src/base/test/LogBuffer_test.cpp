#include "../log/LogBuffer.hpp"

#include <stdio.h>

using namespace fver::base;

int main() {
  log::LogBuffer buf;
  buf.Push("1123");
  buf.Push("1231231");
  printf("logbuffer ptr = %s\n", buf.GetBufferPtr());
  return 0;
}