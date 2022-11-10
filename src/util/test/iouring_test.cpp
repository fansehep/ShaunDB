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
  int fd_2 = ::open("./config.yaml", O_RDONLY);
  if (fd_2 <= 0) {
    LOG_ERROR("open fd_2 error");
  }
  char buf_2[1024];
  ReadRequest read_request_2(fd_2, buf_2, sizeof(buf_2));
  io_uring.PrepRead(&read_request_2);
  //
  io_uring.Submit();
  auto con_que = io_uring.WaitFinishQueue();
  //auto con_que_2 = io_uring.WaitFinishQueue();
  //
  //
  //
  LOG_INFO("read_buf: {}", buf);
  LOG_INFO("read fd_2 buf: {}", buf_2);
  // 消除事件
  io_uring.DeleteEvent(&con_que);
  //
  io_uring.Stop();
  ::close(fd_2);
  ::close(fd);
}


}  // namespace iouring

}  // namespace util

}  // namespace fver