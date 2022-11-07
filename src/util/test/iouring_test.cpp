#include "src/util/iouring.hpp"
#include "src/base/log/logging.hpp"

extern "C" {
#include <fcntl.h>
}
#include <thread>
#include <gtest/gtest.h>

namespace fver {

namespace util {

namespace iouring {

class IOUringTest : public ::testing::Test {
 public:
  IOUringTest() = default;
  ~IOUringTest() = default;
  void SetUp() override {}
  void TearDown() override {}
};


TEST_F(IOUringTest, io_uring_write_file_test) {
  IOUring io_uring;
  io_uring.Init(12);
  int fd = ::open("./append_file_test.log", O_RDONLY);
  if (fd <= 0) {
    LOG_ERROR("open file error");
  }
  char buf[102400] = {0};
  ReadRequest read_request(fd, buf, sizeof(buf));
  io_uring.PrepRead(&read_request);
  io_uring.Submit();
  auto con_que = io_uring.WaitFinishQueue();
  //
  //
  //
  LOG_INFO("read_buf: {}", buf);
  // 消除事件
  io_uring.DeleteEvent(&con_que);
  //
  io_uring.Stop();
  ::close(fd);
}


}  // namespace iouring

}  // namespace util

}  // namespace fver