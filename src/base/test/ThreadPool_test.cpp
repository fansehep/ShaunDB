#include "../ThreadPool.hpp"

#include <fmt/format.h>

#include <iostream>

#include "../log/Logging.hpp"

using namespace fver::base;

void Print() {
  fmt::print("hello, world!");
  LOG_INFO("this is fver!");
}

int main() {
  ThreadPool pool(2);
  pool.AddTask(Print);
  pool.AddTask(Print);
  pool.Start();
  pool.Stop();
  return 0;
}