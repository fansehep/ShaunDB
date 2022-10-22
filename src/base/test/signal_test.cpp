#include <fmt/format.h>
extern "C" {
#include <sys/signal.h>
}
#include <chrono>
#include <iostream>
#include <thread>

#include "src/base/backtrace.hpp"

void SignalExecute(int signal) {
  switch (signal) {
    case SIGINT: {
      fmt::print("SIGNAL: \n");
      auto error_log = fver::base::stackTrace(true);
      fmt::print("{}\n", error_log);
      exit(-1);
    }
    case SIGQUIT: {
      fmt::print("SIGNAL: \n");
      auto error_log = fver::base::stackTrace(true);
      fmt::print("{}\n", error_log);
      exit(-1);
    }
  }
}

int main() {
  signal(SIGINT, SignalExecute);
  signal(SIGQUIT, SignalExecute);
  while (1) {
    int a = 10;
    printf("a = %d\n", a);
    std::cout << "a = " << a << std::endl;
    fmt::print("a = {}\n", a);
  }
  return 0;
}
