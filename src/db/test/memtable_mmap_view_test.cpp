#include <gtest/gtest.h>

#include <thread>
#include <unordered_map>

#include "src/db/exportdb.hpp"
#include "src/db/memtable_view.hpp"
#include "src/db/request.hpp"
#include "src/db/status.hpp"

extern "C" {
#include <uuid/uuid.h>
}

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

namespace fver {

namespace db {

class MemTableViewTest : public ::testing::Test {
 public:
  MemTableViewTest() = default;
  ~MemTableViewTest() = default;
  void SetUp() override {}
  void TearDown() override {}
};

struct GetCallBackCompare {
 public:
  std::string_view value;
  void CompareEQ(const std::shared_ptr<GetContext>& get_context) {
    ASSERT_EQ(value, get_context->value);
    ASSERT_EQ(get_context->code.getCode(), StatusCode::kOk);
    assert(false);
    delete this;
  }
  void CompareNotExist(const std::shared_ptr<GetContext>& get_context) {
    ASSERT_EQ(get_context->code.getCode(), StatusCode::kNotFound);
    assert(false);
    delete this;
  }
  void CompareDelete(const std::shared_ptr<GetContext>& get_context) {
    ASSERT_EQ(get_context->code.getCode(), StatusCode::kDelete);
    assert(false);
    delete this;
  }
};

// 90 秒的测试
TEST_F(MemTableViewTest, simple_view_test) {
  struct DBConfig db_config;
  db_config.wal_log_size = 64 * 1024 * 1024;
  db_config.db_path = "./test";
  db_config.memtable_N = 1;
  // for test 4mb 一次 minor_compaction
  db_config.memtable_trigger_size = 1024 * 1024 / 8;
  db_config.compactor_thread_size = 1;
  db_config.isRecover = false;
  DB db;
  db.Init(db_config);
  const int curreny_N = 16;
  std::vector<std::thread> workers;
  int i = 0;
  static thread_local char key[64] = {0};
  static thread_local char value[64] = {0};
  static thread_local uuid_t uuid_key;
  static thread_local uuid_t uuid_value;

  static thread_local char test_key[64] = {0};
  static thread_local uuid_t test_uuid;

  std::unordered_map<std::string, std::string> origin_test_map;
  std::mutex origin_mtx_;

  bool isRunning = true;
  uint32_t j = 0;
  uint32_t a;
  LOG_INFO("start insert");
  for (; i < curreny_N; i++) {
    workers.emplace_back([&]() -> void {
      while (true == isRunning) {
        auto set_context = std::make_shared<SetContext>();
        uuid_generate(uuid_key);
        uuid_generate(uuid_value);
        uuid_unparse(uuid_key, key);
        uuid_unparse(uuid_value, value);
        set_context->key = std::string_view(key, 36);
        set_context->value = std::string_view(value, 36);
        uuid_generate(test_uuid);
        uuid_unparse(test_uuid, test_key);
        auto compre = new (std::nothrow) GetCallBackCompare();
        auto get_context = std::make_shared<GetContext>();
        get_context->key = test_key;
        db.Set(set_context);
        origin_mtx_.lock();
        origin_test_map[set_context->key] = set_context->value;
        auto iter = origin_test_map.find(test_key);
        a = rand() % origin_test_map.size();
        for (auto& iter : origin_test_map) {
          j++;
          if (j == a) {
            auto compre_t = new (std::nothrow) GetCallBackCompare();
            auto get_context_2 = std::make_shared<GetContext>();
            get_context_2->key = iter.first;
            compre_t->value = iter.second;
            get_context_2->get_callback =
                std::bind(&GetCallBackCompare::CompareEQ, compre_t, _1);
            db.Get(get_context_2);
            break;
          }
        }
        j = 0;
        origin_mtx_.unlock();
        // 不存在
        if (iter == origin_test_map.end()) {
          // 使用不存在的回调函数
          get_context->get_callback =
              std::bind(&GetCallBackCompare::CompareNotExist, compre, _1);
        } else {
          // 使用应该相等的回调函数
          compre->value = iter->second;
          get_context->get_callback =
              std::bind(&GetCallBackCompare::CompareEQ, compre, _1);
        }
        db.Get(get_context);
      }
    });
  }
  std::this_thread::sleep_for(std::chrono::seconds(1024));
  isRunning = false;
  for (auto& iter : workers) {
    iter.join();
  }
}

}  // namespace db

}  // namespace fver