#include "src/util/klrucache.hpp"

#include <gtest/gtest.h>

#include "src/base/log/logging.hpp"

extern "C" {
#include <uuid/uuid.h>
}

namespace fver {

namespace util {

class kLRUCacheTest : public ::testing::Test {
 public:
  kLRUCacheTest() = default;
  ~kLRUCacheTest() = default;
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(kLRUCacheTest, onemillion_test) {
  // 热数据使用 4MB 空间
  const uint32_t hot_memory_usage = 128 * 4 * 1024;
  // 冷数据使用 1MB 空间
  const uint32_t cold_memory_usage = 128 * 1024;
  std::unordered_map<std::string, std::string> test_map;
  kLRUCache klrucache(cold_memory_usage, hot_memory_usage);
  uuid_t uuid_key;
  uuid_t uuid_value;
  char key[64];
  char value[64];
  int i = 0;
  const int N = 10000;
  for (; i < N; i++) {
    uuid_generate(uuid_key);
    uuid_unparse(uuid_key, key);
    uuid_generate(uuid_value);
    uuid_unparse(uuid_value, value);
    klrucache.set(key, value);
    test_map[key] = value;
  }
  LOG_INFO("read test");
  std::string v;
  bool isErase = false;
  int j = 0;
  for (auto& iter : test_map) {
    auto ue = klrucache.get(iter.first, &v, &isErase);

    if (ue == false) {
      LOG_DEBUG("lrucache key: {} can't found", iter.first);
      continue;
    }
    if (isErase == false) {
      ASSERT_EQ(v, iter.second);
      ASSERT_EQ(isErase, false);
    }
    LOG_TRACE("get value: {} equal iter.second: {} {}", v, iter.second, i++);
  }
}

TEST_F(kLRUCacheTest, cold_hot_data_test) {
  // 热数据使用 4MB 空间
  const uint32_t hot_memory_usage = 4 * 1024;
  // 冷数据使用 1MB 空间
  const uint32_t cold_memory_usage = 1 * 1024;
  std::unordered_map<std::string, std::string> test_map;
  kLRUCache klrucache(cold_memory_usage, hot_memory_usage);
  uuid_t uuid_key;
  uuid_t uuid_value;
  char key[64];
  char value[64];
  int i = 0;
  const int N = 10000;
  std::string pre_key;
  std::string pre_value;
  bool isExist;
  for (; i < N; i++) {
    uuid_generate(uuid_key);
    uuid_unparse(uuid_key, key);
    uuid_generate(uuid_value);
    uuid_unparse(uuid_value, value);
    klrucache.set(key, value);
    LOG_INFO("set ok!");
    pre_key = key;
    klrucache.get(pre_key, &pre_value, &isExist);
    ASSERT_EQ(value, pre_value);
    LOG_INFO("klrucache hot_size: {} cold_size: {}", klrucache.getMemsize(),
             klrucache.getMaxMemsize());
    test_map[key] = value;
  }
  std::string v;
  bool isErase = false;
  for (auto& iter : test_map) {
    auto ue = klrucache.get(iter.first, &v, &isErase);
    if (ue == false) {
      LOG_DEBUG("lrucache key: {} can't found", iter.first);
      continue;
    }
    if (isErase == false) {
      ASSERT_EQ(v, iter.second);
      ASSERT_EQ(isErase, false);
    }
    LOG_TRACE("get value: {} equal iter.second: {} {}", v, iter.second, i++);
  }
}

}  // namespace util

}  // namespace fver