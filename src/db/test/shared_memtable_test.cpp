#include "src/db/shared_memtable.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <thread>

#include "src/base/log/logging.hpp"
#include "src/db/request.hpp"

extern "C" {
#include <uuid/uuid.h>
}

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

namespace fver {

namespace db {

class SharedMemtableTest : public ::testing::Test {
 public:
  SharedMemtableTest() = default;
  ~SharedMemtableTest() = default;
  void SetUp() override {}
  void TearDown() override {}
};

struct CallBackCompare {
  std::string value;
  void Compare(const std::shared_ptr<GetContext>& get_context) {
    ASSERT_EQ(value, get_context->value);
    delete this;
  }
};

// 简单的插入, 验证 varint 是否正确
TEST_F(SharedMemtableTest, simple_insert_tset) {
  SharedMemtable table;
  // 默认一个 SharedMemtable 拥有 4 个 memTable
  table.Init(8, 256 * 1024 * 1024);
  table.Run();
  auto set_context_1 = std::make_shared<SetContext>();
  const std::string key_1 = "123123";
  const std::string value_1 = "12dajfiljdsklf";
  set_context_1->key = key_1;
  set_context_1->value = value_1;

  auto set_context_2 = std::make_shared<SetContext>();
  const std::string key_2 = "djakfhn";
  const std::string value_2 = "cjsiklfhsuidkf";
  set_context_2->key = key_2;
  set_context_2->value = value_2;
  table.Set(set_context_1);
  table.Set(set_context_2);

  auto get_context_1 = std::make_shared<GetContext>();
  get_context_1->key = key_1;
  auto compare = new (std::nothrow) CallBackCompare;
  compare->value = value_1;
  get_context_1->get_callback =
      std::bind(&CallBackCompare::Compare, compare, _1);

  auto get_context_2 = std::make_shared<GetContext>();
  get_context_2->key = key_2;
  auto compare_2 = new (std::nothrow) CallBackCompare;
  compare_2->value = value_2;
  get_context_2->get_callback =
      std::bind(&CallBackCompare::Compare, compare_2, _1);
  std::this_thread::sleep_for(std::chrono::seconds(1));
}

// 多线程插入
// 验证正确性
// 29993
TEST_F(SharedMemtableTest, Multi_thread_insert) {
  SharedMemtable table;
  // 默认一个 SharedMemtable 拥有 4 个 memTable
  table.Init(4, 256 * 1024 * 1024);
  table.Run();
  // 8 个线程同时插入
  const int curreny_N = 8;
  std::vector<std::thread> workers_;

  std::mutex origin_map_mtx;
  std::map<std::string, std::string> origin_map_test;

  int i = 0;
  static thread_local char key[64] = {0};
  static thread_local char value[64] = {0};
  static thread_local uuid_t uuid_key;
  static thread_local uuid_t uuid_value;
  LOG_INFO("start insert");
  for (; i < curreny_N; i++) {
    workers_.emplace_back([&]() {
      for (int i = 0; i < 100000; i++) {
        auto set_context = std::make_shared<SetContext>();
        uuid_generate(uuid_key);
        uuid_generate(uuid_value);
        uuid_unparse(uuid_key, key);
        uuid_unparse(uuid_value, value);
        set_context->key = std::string_view(key, 36);
        set_context->value = std::string_view(value, 36);
        origin_map_mtx.lock();
        origin_map_test[set_context->key] = set_context->value;
        origin_map_mtx.unlock();
        table.Set(set_context);
      }
    });
  }

  for (auto& iter : workers_) {
    if (iter.joinable()) {
      iter.join();
    }
  }
  LOG_INFO("multi thread start check");
  i = 0;
  for (auto& [key, value] : origin_map_test) {
    auto get_context = std::make_shared<GetContext>();
    get_context->key = key;
    auto compare = new (std::nothrow) CallBackCompare;
    assert(compare != nullptr);
    compare->value = value;
    get_context->get_callback =
        std::bind(&CallBackCompare::Compare, compare, _1);
    table.Get(get_context);
  }
}

}  // namespace db

}  // namespace fver