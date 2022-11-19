#include <gtest/gtest.h>
#include "src/db/memtable_view.hpp"
#include "src/db/exportdb.hpp"

extern "C" {
#include <uuid/uuid.h>
}



namespace fver {


namespace db {

class MemTableViewTest : public ::testing::Test {
public:
  MemTableViewTest() = default;
  ~MemTableViewTest() = default;
  void SetUp() override {}
  void TearDown() override {}
};

//TODO
TEST_F(MemTableViewTest, simple_view_test) {
  struct DBConfig db_config;
  db_config.wal_log_size = 64 * 1024 * 1024;
  db_config.db_path = "./test";
  db_config.memtable_N = 1;
  db_config.memtable_trigger_size = 1024 * 16;
  db_config.memtable_trigger_size = 
}





}






}