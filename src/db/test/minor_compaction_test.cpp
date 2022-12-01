#include <gtest/gtest.h>

extern "C" {
#include <uuid/uuid.h>
}

#include <absl/container/flat_hash_map.h>

#include "src/base/log/logging.hpp"
#include "src/db/exportdb.hpp"

namespace fver {

namespace db {

using std::placeholders::_1;

std::atomic<uint64_t> g_N = 0;
struct CallBackCompare {
  std::string value;
  void Compare(const std::shared_ptr<GetContext>& get_context) {
    if (value != get_context->value) {
      LOG_ERROR("value: {} != get_context_value: {} code_status: {}", value,
                get_context->value, get_context->code.getCodeStr());
      assert(false);
      exit(-1);
    }
    g_N++;
    LOG_INFO("g_N: {} v: {} value: {}", g_N, get_context->value, value);
    ASSERT_EQ(value, get_context->value);
    delete this;
  }
};

class MinorCompactionTest : public ::testing::Test {
 public:
  MinorCompactionTest() = default;
  ~MinorCompactionTest() = default;
  void SetUp() override {
    // ::system("rm -r ./test/*");
  }
  void TearDown() override {
    // ::system("rm -r ./test/*");
  }
};

TEST_F(MinorCompactionTest, simple_compaction_test) {
  struct DBConfig config;
  config.wal_log_size = 64 * 1024 * 1024;
  config.db_path = "./test";
  config.memtable_N = 1;
  // 256 byte 直接触发 minor_compaction
  config.memtable_trigger_size = 1024 * 1024 / 12;
  config.compactor_thread_size = 1;
  config.isRecover = false;
  config.max_level_size = 8;
  DB db;
  std::unordered_map<std::string, std::string> test_map;
  std::mutex test_mtx_;
  db.Init(config);
  // 16 个线程同时插入
  const int curreny_N = 16;
  std::vector<std::thread> workers_;

  int i = 0;
  static thread_local char key[64] = {0};
  static thread_local char value[64] = {0};
  static thread_local uuid_t uuid_key;
  static thread_local uuid_t uuid_value;
  LOG_INFO("start insert");
  bool isRunning = true;
  for (; i < curreny_N; i++) {
    workers_.emplace_back([&]() {
      while (true == isRunning) {
        auto set_context = std::make_shared<SetContext>();
        uuid_generate(uuid_key);
        uuid_generate(uuid_value);
        uuid_unparse(uuid_key, key);
        uuid_unparse(uuid_value, value);
        set_context->key = std::string_view(key, 36);
        set_context->value = std::string_view(value, 36);
        test_mtx_.lock();
        test_map[set_context->key] = set_context->value;
        test_mtx_.unlock();
        db.Set(set_context);
      }
    });
  }
  //
  std::this_thread::sleep_for(std::chrono::seconds(360));
  isRunning = false;
  for (auto& iter : workers_) {
    iter.join();
  }
  uint64_t g_nn = 0;
  LOG_INFO("test_map_size: {}", test_map.size());
  std::thread find_thread([&]() -> void {
    for (auto& iter : test_map) {
      g_nn++;
      auto get_context = std::make_shared<GetContext>();
      get_context->key = iter.first;
      auto cann = new CallBackCompare();
      cann->value = iter.second;
      get_context->get_callback =
          std::bind(&CallBackCompare::Compare, cann, _1);
      db.Get(get_context);
      LOG_TRACE("g_nn : {}", g_nn);
    }
  });
  std::this_thread::sleep_for(std::chrono::seconds(60));
  find_thread.join();
}

}  // namespace db

}  // namespace fver