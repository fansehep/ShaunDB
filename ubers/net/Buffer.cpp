#include <cerrno>
#include <sys/uio.h>
#include "Buffer.h"
#include "SocketOps.h"
using namespace UBERS;
using namespace UBERS::net;

const char Buffer::kCRLF[] = "\r\n";

const size_t Buffer::kPrepend;
const size_t Buffer::kInitSize;

//结合栈上的空间，避免内存使用过大，提高内存使用率
//如果有5k个连接，每个连接就分配64k+64k的缓冲区的话，将占用640m
//而大多数的时候，这些缓冲区的使用率很低
ssize_t Buffer::readFd(int fd, int *savedErrno)
{
    //节省了一次ioctl系统调用（获取有多少可读数据）
    char extrabuf[65536];
    struct iovec vec[2];
    const size_t writable = writeableBytes();
    //第一块缓冲区
    vec[0].iov_base = begin() + writerIndex_;
    vec[0].iov_len = writable;
    //第二块缓冲区
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof extrabuf;
    const int iovcnt = (writable < sizeof extrabuf) ? 2 : 1;
    const ssize_t n = sockets::Readv(fd, vec, iovcnt);
    if (n < 0)
    {
        *savedErrno = errno;
    }
    else if (static_cast<size_t>(n) <= writable) //第一块缓冲区足够容纳
    {
        writerIndex_ += static_cast<size_t>(n);
    }
    else //当前缓冲区不够容纳，所以数据被传到了第二块缓冲区extrabuf，将其append到buffer
    {
        writerIndex_ = buffer_.size();
        append(extrabuf, static_cast<size_t>(n) - writable);
    }
    return n;
}