#include "src/db/shared_memtable.hpp"

#include <gtest/gtest.h>

extern "C" {
#include <uuid/uuid.h>
}

namespace fver {

namespace db {

class SharedMemtableTest : public ::testing::Test {
 public:
  SharedMemtableTest() = default;
  ~SharedMemtableTest() = default;
  void SetUp() override {}
  void TearDown() override {}
};

// 多线程插入
// 验证正确性
TEST_F(SharedMemtableTest, Multi_thread_insert) {
  SharedMemtable table;
  // 8 个线程同时插入
  const int curreny_N = 8;
  std::vector<std::thread> workers_;
  std::map<std::string, std::string> origin_map_test;
  int i = 0;
  static thread_local char key[64] = {0};
  static thread_local char value[64] = {0};
  static thread_local uuid_t uuid_key;
  static thread_local uuid_t uuid_value;
  static thread_local auto set_context = std::make_shared<SetContext>();
  for (; i < curreny_N; i++) {
    workers_.emplace_back([&]() {
      uuid_generate(uuid_key);
      uuid_generate(uuid_value);
      uuid_unparse(uuid_key, key);
      uuid_unparse(uuid_value, value);
      set_context->key = key;
      set_context->value = value;
      origin_map_test[key] = value;
      
    });
  }
}

}  // namespace db

}  // namespace fver