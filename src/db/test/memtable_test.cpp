#include "src/db/memtable.hpp"

#include <gtest/gtest.h>

#include "src/db/request.hpp"

extern "C" {
#include <uuid/uuid.h>
}

namespace fver {

namespace db {

class MemtableTest : public ::testing::Test {
 public:
  MemtableTest() = default;
  ~MemtableTest() = default;
  void SetUp() override {}
  void TearDown() override {}
};

// 单线程插入, 检测线程安全 与 正确性
// >_< memtable 非线程安全
TEST_F(MemtableTest, one_thread_insert) {
  // 同时插入, 最后验证正确性
  std::map<std::string, std::string> origin_test_map;
  Memtable table;
  const int curreny_N = 1000;
  int i = 0;
  char key[64] = {0};
  char value[64] = {0};
  uuid_t uuid_key;
  uuid_t uuid_value;
  auto set_context = std::make_shared<SetContext>();
  for (; i < curreny_N; i++) {
    uuid_generate(uuid_key);
    uuid_generate(uuid_value);
    uuid_unparse(uuid_key, key);
    uuid_unparse(uuid_value, value);
    set_context->key = key;
    set_context->value = value;
    set_context->number = i;
    table.Set(set_context);
    origin_test_map[set_context->key] = set_context->value;
  }
  auto get_context = std::make_shared<GetContext>();
  for (auto [key, value] : origin_test_map) {
    get_context->key = key;
    table.Get(get_context);
    ASSERT_EQ(get_context->value, value);
  }
}

}  // namespace db

}  // namespace fver