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
      std::string error_log;
      fver::base::backtrace(&error_log);

      fmt::print("SIGNAL: {}\n", error_log);
      void* stack[32];
      int depth = fver::base::GetStackTrace(stack, 32, int skip_count)
      exit(-1);
      break;
    }
    case SIGQUIT: {
      std::string error_log;
      fver::base::backtrace(&error_log);

      fmt::print("SIGNAL: {}\n", error_log);

      exit(-1);
      break;
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
