#include <gtest/gtest.h>

extern "C" {
#include <uuid/uuid.h>
}

#include "src/base/log/logging.hpp"
#include "src/db/exportdb.hpp"

namespace fver {

namespace db {

class MinorCompactionTest : public ::testing::Test {
 public:
  MinorCompactionTest() = default;
  ~MinorCompactionTest() = default;
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(MinorCompactionTest, simple_compaction_test) {
  struct DBConfig config;
  config.wal_log_size = 64 * 1024 * 1024;
  config.db_path = "./test";
  config.memtable_N = 16;
  // 256 byte 直接触发 minor_compaction
  config.memtable_trigger_size = 1024 * 1024 / 12;
  config.compactor_thread_size = 1;
  config.isRecover = false;
  DB db;
  db.Init(config);
  // 80 个线程同时插入
  const int curreny_N = 8;
  std::vector<std::thread> workers_;

  int i = 0;
  static thread_local char key[64] = {0};
  static thread_local char value[64] = {0};
  static thread_local uuid_t uuid_key;
  static thread_local uuid_t uuid_value;
  LOG_INFO("start insert");
  for (; i < curreny_N; i++) {
    workers_.emplace_back([&]() {
      for (int i = 0; i < 1000000; i++) {
        auto set_context = std::make_shared<SetContext>();
        uuid_generate(uuid_key);
        uuid_generate(uuid_value);
        uuid_unparse(uuid_key, key);
        uuid_unparse(uuid_value, value);
        set_context->key = std::string_view(key, 36);
        set_context->value = std::string_view(value, 36);
        db.Set(set_context);
      }
    });
  }
  //
  for (auto& iter : workers_) {
    iter.join();
  }
  std::this_thread::sleep_for(std::chrono::seconds(1000));
}

}  // namespace db

}  // namespace fver