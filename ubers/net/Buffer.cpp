#include <errno.h>
#include <sys/uio.h>
#include "Buffer.h"
#include "SocketOps.h"

using namespace UBERS;
using namespace UBERS::net;

const size_t Buffer::kPrepend;
const size_t Buffer::kInitSize;

size_t Buffer::ReadFd(int fd, int* saveErrno)
{

  char ExtraBuf[65536];
  struct iovec vec[2];
  const size_t Writable = WriteableBytes();
  // 第一块缓冲区
  vec[0].iov_base = begin() + this->writerIndex_;
  vec[0].iov_len = Writable;
  // 第二块缓冲区
  vec[1].iov_base = ExtraBuf;
  vec[1].iov_len = sizeof(ExtraBuf);

  const int iovcnt = (Writable < sizeof(ExtraBuf)) ? 2 : 1;
  const ssize_t n = socketops::Readv(fd, vec, iovcnt);
  if(n < 0)
  {
    *saveErrno = errno;
  }
  else if(static_cast<size_t>(n) <= Writable)
  {
    this->writerIndex_ += static_cast<size_t>(n);
  }
  else
  {
    this->writerIndex_ = buffer_.size();
    Append(ExtraBuf, static_cast<size_t>(n) - Writable);
  }
  return n;
}