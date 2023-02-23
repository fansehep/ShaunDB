#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

#include <chrono>
#include <thread>
#include <vector>

#include <benchmark/benchmark.h>

auto spdlog_to_file() {
  try {
    auto logger = spdlog::basic_logger_mt(
        "basic_logger", "./benchmark/logbenchmark/spdlog_bench.log");
    return logger;
  } catch (const spdlog::spdlog_ex& ex) {
    fmt::print("init spdlog logger error!");
  }
}

static void TEST_SPDLOG_TO_FILE_NOSYNC(benchmark::State& state) {
  fmt::print("test spdlog init!\n");
  auto spd_log = spdlog_to_file();
  for (auto _ : state) {
    const int corrunyN = std::thread::hardware_concurrency();
    std::vector<std::thread> workers;
    bool isRunning = true;
    int i = 0;
    for (; i < corrunyN; i++) {
      workers.emplace_back([&]() {
        while (isRunning) {
          spd_log->info("I want to fly in the sky, {}, {}, {}, {}", rand(),
                        rand(), rand(), std::to_string(rand()));
        }
      });
    }
    std::this_thread::sleep_for(std::chrono::seconds(30));
    isRunning = false;
    for (auto& thid : workers) {
      thid.join();
    }
  }
}

BENCHMARK(TEST_SPDLOG_TO_FILE_NOSYNC);

BENCHMARK_MAIN();
