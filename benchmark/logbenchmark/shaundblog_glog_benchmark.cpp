#include <benchmark/benchmark.h>
#include <glog/logging.h>

#include <chrono>
#include <thread>
#include <vector>

#include "src/base/log/logging.hpp"


/*
 * fverlog vs glog
 * 编译参数详见 fver/.coptsbzl
 * glog version: 0.6.0-1
 * fverlog version: latest
 * google-benchmark version: 1.12.1-1
 */

// fverlog test no sync
// 使用 4 个线程并发写入文件, 不启用 sync 模式
// 且启用最低 INFO 日志等级
// 3 个随机数, 1个随机数的 to_string
// 测试对比 10s 内谁写入的日志数量最多
// 本机测试 一般fverlog 的相同时间写入量是 glog 的 20 倍
static void TEST_FLOG_TO_FILE_NOSYNC(benchmark::State& state) {
  fver::base::log::Init("./benchmark/logbenchmark",
                        kLogLevel::kInfo, false, "test");
  for (auto _ : state) {
    const int corrunyN = std::thread::hardware_concurrency();
    std::vector<std::thread> workers;
    bool isRunning = true;
    int i = 0;
    for (; i < corrunyN; i++) {
      workers.emplace_back([&]() {
        while (isRunning) {
          LOG_INFO("I want to fly in the sky, {}, {}, {}, {}", rand(), rand(),
                   rand(), std::to_string(rand()));
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

// 18471585
// 25388906


BENCHMARK(TEST_FLOG_TO_FILE_NOSYNC);

BENCHMARK_MAIN();
