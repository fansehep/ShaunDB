#include "src/util/iouring.hpp"

#include "src/base/log/logging.hpp"

namespace fver {

namespace util {

bool IOUring::Init(const uint32_t ring_depth) {
  ring_depth_ = ring_depth;
  // TODO: 考虑不同的选择方式
  auto init_ue = ::io_uring_queue_init(ring_depth_, &ring_, 0);
  // io_uring_queue_init 成功返回 0
  if (init_ue != 0) {
    LOG_ERROR("io_uring init error: {}", ::strerror(errno));
    assert(false);
    return false;
  }
  return true;
}

bool IOUring::Stop() {}

static thread_local struct ::io_uring_sqe* sqe = nullptr;

uint32_t IOUring::PrepReadv(IOUringReadRequest* request) {
  int i = 0;
  int read_offset = 0;
  for (; i < request->iovec_data_size_; i++) {
    sqe = ::io_uring_get_sqe(&ring_);
    ::io_uring_prep_readv(sqe, request->fd_, &(request->iovec_data_[i]), 1,
                          request->iovec_offset_);
    read_offset += (request->iovec_data_[i].iov_len);
  }
}

}  // namespace util

}  // namespace fver
