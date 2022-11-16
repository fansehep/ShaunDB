#include <gtest/gtest.h>

extern "C" {
#include <uuid/uuid.h>
}
#include "src/db/sstable.hpp"
#include "src/util/iouring.hpp"
#include "src/base/log/logging.hpp"

namespace fver {

namespace db {

class SSTableIOUringTest : public ::testing::Test {
 public:
  SSTableIOUringTest() = default;
  ~SSTableIOUringTest() = default;
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(SSTableIOUringTest, simpel_write_test) {
  util::iouring::IOUring io_uring;
  io_uring.Init(1024);
  SSTable sstable;
  sstable.Init("./test", "test.sst", 0, 0);
  char data[] = "hello sstable i want to fly";
  util::iouring::WriteRequest write_request(sstable.getFd(), data,
                                            ::strlen(data));
  LOG_INFO("io_uring prep write");
  io_uring.PrepWrite(&write_request);
  LOG_INFO("io_uring submit");
  io_uring.Submit();
  LOG_INFO("io_uring wait finish queue");
  auto conn = io_uring.WaitFinishQueue();
  if (conn.getData() == data) {
    LOG_INFO("delete data");
  }
  io_uring.Stop();
}

}  // namespace db

}  // namespace fver