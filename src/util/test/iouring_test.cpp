#include "src/util/iouring.hpp"

#include "src/base/log/logging.hpp"

extern "C" {
#include <fcntl.h>
}
#include <gtest/gtest.h>

#include <thread>

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
  char buf[100] = {0};
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
  // auto con_que_2 = io_uring.WaitFinishQueue();
  //
  //
  //
  LOG_INFO("read_buf: {}", buf);
  LOG_INFO("read fd_2 buf: {}", buf_2);
  // 消除事件
  io_uring.DeleteEvent(&con_que);
  auto conn_2 = io_uring.WaitFinishQueue();
  io_uring.DeleteEvent(&conn_2);
  //
  auto conn_3 = io_uring.PeekFinishQueue();
  if (conn_3.isEmpty()) {
    LOG_INFO("conn_3 == nullptr");
  }
  ::close(fd_2);
  ::close(fd);
}

TEST_F(IOUringTest, io_uring_set_flag_read_test) {
  IOUring io_uring;
  io_uring.Init(12);
  int fd = ::open("./append_file_test.log", O_RDONLY);
  if (fd <= 0) {
    LOG_ERROR("open file error");
  }
  char buf[100] = {0};
  ReadRequest read_request(fd, buf, sizeof(buf));
  io_uring.PrepRead(&read_request);
  int fd_2 = ::open("./config.yaml", O_RDONLY);
  if (fd_2 <= 0) {
    LOG_ERROR("open fd_2 error");
  }
  char buf_2[1024] = {0};
  ReadRequest read_request_2(fd_2, buf_2, sizeof(buf_2));
  io_uring.PrepRead(&read_request_2);
  io_uring.Submit();
  auto con = io_uring.WaitFinishQueue();
  if (con.getData() == buf) {
    LOG_INFO("config.yaml: {}", buf_2);
  } else if (con.getData() == buf_2) {
    LOG_INFO("append_file_test.log: {}", buf);
  }
  LOG_INFO("here?");
  io_uring.DeleteEvent(&con);
  auto con_2 = io_uring.WaitFinishQueue();
  if (con_2.getData() == buf) {
    LOG_INFO("config.yaml: {}", buf_2);
  } else if (con_2.getData() == buf_2) {
    LOG_INFO("append_file_test.log: {}", buf);
  }
  io_uring.DeleteEvent(&con_2);
  io_uring.Stop();
  ::close(fd_2);
  ::close(fd);
}

TEST_F(IOUringTest, io_uring_write_test) {
  // IOUring io_uring;
  // io_uring.Init(12);
  // int fd = ::open("./test/io_uring_test.log");
}



}  // namespace iouring

}  // namespace util

}  // namespace fver