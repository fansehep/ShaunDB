#include <atomic>
#include <chrono>
#include <thread>
#include <vector>

#include "src/base/log/logging.hpp"

std::atomic<uint64_t> sum(0);

void Test_TO_STD_OUT() {
  std::vector<std::thread> workers;
  int i = 0;
  bool running = true;
  for (; i < 5; i++) {
    workers.emplace_back([&]() {
      while (running) {
        LOG_INFO("I Love Rust or {}", "Go");
      }
    });
  }
  std::this_thread::sleep_for(std::chrono::seconds(3));
  running = false;
  for (auto& iter : workers) {
    iter.join();
  }
}

void Test_TO_FILE() {
  fver::base::log::Init("/home/fan/GitHub/fver/src/base/test",
                        kLogLevel::kInfo, false, "test");
  std::vector<std::thread> workers;
  int i = 0;
  bool running = true;

  for (; i < 5; i++) {
    workers.emplace_back([&]() {
      while (running) {
          LOG_INFO("I Love Rust or {}", "Go");
          sum.fetch_add(1);
          fmt::print("sum_write = {}\n", sum.load());
      }
    });
  }

  for (int i = 0; i < 100; i ++) {
    LOG_INFO("I Love Rust or {} 1", "Go");
  }
  std::this_thread::sleep_for(std::chrono::seconds(30));
  running = false;
  for (auto& iter : workers) {
    iter.join();
  }
  std::this_thread::sleep_for(std::chrono::seconds(30));
  LOG_INFO("Sum = {}", sum);
  std::this_thread::sleep_for(std::chrono::seconds(5));
}

int main() {
  Test_TO_FILE();
  return 0;
}