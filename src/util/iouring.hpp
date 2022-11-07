#ifndef SRC_UTIL_IO_URING_H_
#define SRC_UTIL_IO_URING_H_

#include <liburing.h>

#include "src/base/noncopyable.hpp"

using ::fver::base::NonCopyable;

namespace fver {

namespace util {

struct IOUringReadRequest {
  // 需要读取数据的 fd
  int fd_;
  // 读数据请求
  struct ::iovec* iovec_data_;
  // iovec_data_ 的容量
  uint32_t iovec_data_size_;
  // 读取操作的 偏移量
  uint32_t iovec_offset_;
  /*
    @file_fd: 需要读取数据的 fd
    @data_size: 需要读取数据的 容量
  */
  void Init(const int file_fd, const uint32_t data_size);
};

class IOUring : public NonCopyable {
 public:
  IOUring() = default;
  /*
    @ring_depth: io_uring 的队列长度
  */
  bool Init(const uint32_t ring_depth);
  /*
    停止一个 io_uring 实例
  */
  bool Stop();
  /*
    @request: 准备读取请求
  */
  uint32_t PrepReadv(IOUringReadRequest* request);

  /*
    提交请求
  */
  uint32_t Submit();

  uint32_t WaitRequest();

 private:
  // io_uring 的队列长度
  uint32_t ring_depth_;
  // io_uring 句柄
  struct ::io_uring ring_;
};

}  // namespace util

}  // namespace fver

#endif