#ifndef SRC_UTIL_IO_URING_H_
#define SRC_UTIL_IO_URING_H_

#include <liburing.h>

#include "src/base/noncopyable.hpp"

using ::fver::base::NonCopyable;

namespace fver {

namespace util {

namespace iouring {

// io_uring wirte: no_block
/*
 * @ struct io_uring_sqe : 提交队列
 * @ int fd : 需要操作的 fd
 * @ const void* buf : 需要写入的数据
 * @ unsigned int nbytes : 数据总量
 * @ __u64 offset : 文件偏移量, if -1 == offset, 那么写操作将会从文件偏移量开始
 * @ return : 失败时同步 return -1, liburing 会将 errno 设置为错误值
 */
// int io_uring_prep_write(struct io_uring_sqe *sqe, int fd, const void *buf,
// unsigned int nbytes, __u64 offset)

// io_uring read: no_block
/*
 * @ struct io_uring_sqe : 提交队列
 * @ int fd : 需要读取的 fd
 * @ const void* buf : 需要读取的数据的指针
 * @ unsigned nbytes : 传入 buf size
 * @ __u64 offset : 文件偏移量, if -1 == offset, 那么
 * 读取操作将会从文件偏移量开始
 * @ return : 失败时同步return -1, liburing 将会将 errno 设置为错误值
 */
// int io_uring_prep_read(struct io_uring_sqe* sqe, int fd, void* buf, unsigned
// nbytes, __u64 offset);

// io_uring submit no_block
/*
 * @ struct io_uring* ring : 提交队列
 * @ return : 成功返回当前提交的队列条目数量,
 *            如果失败, 就返回 -errno.
 */

// io_uring peek_cqe no_block
/*
 * @ struct io_uring* ring : io_uring 句柄
 * @ struct io_uring_cqe** cqe_ptr : 输出参数, 是指 cqe 的地址
 * @ return : io_uring_peek_cqe 函数从属于 ring_param 的队列中返回一个
 *            IO 完成,
 *          @ success : 返回 0, 并且 io_uring_cqe 被填充, != nullptr
 *          @ fail : 返回 -EAGAIN.
 */
// int io_uring_peek_cqe(struct io_uring* ring, struct io_uring_cqe** cqe_ptr);

// io_uring wait_cqe
/*
 * @ struct io_uring wait_cqe
 * @ return : 阻塞等待事件完成.
 *
 *
 */
// int io_uring_wait_cqe(struct io_uring* ring, struct io_uring_cqe** cqe_ptr);

// io_uring cqe_seen
/*
 * @ struct io_uring* ring : io_uring 句柄
 * @ struct io_uring_cqe* cqe : 输出参数
 * @ 标记某个事件已经被清除.
 */
//

// TODO: use io_uring_prep_write_fixed();
// and io_uring_prep_read_fiexed();
/*
 *
 * 相比于 io_uring_prep_write
 * buf_index, 指的是从buf哪里开始写入
 void io_uring_prep_write_fixed(struct io_uring_sqe* sqe,
  int fd, const void* buf, unsigned buf_size, off_t offset,
  int buf_index);
 *
*/
struct ReadRequest {
  // 需要读取数据的 fd
  int fd_;
  char* data_;
  uint32_t data_size_;
  // 文件偏移量, -1 使用文件默认偏移量
  int _file_offset_ = -1;
  ReadRequest(const int file_fd, char* data, const uint32_t data_size)
      : fd_(file_fd), data_(data), data_size_(data_size) {}
};

//!!! 考虑 data 的生命周期
// 同步 IO 的优点是阻塞但是可以保证数据及时被读取
// std::shared_ptr<Buffer> shared_buf_;
struct WriteRequest {
  // 需要写入的 fd
  int fd_;
  // 需要写入的数据
  char* data_;
  uint32_t data_size_;
  // 文件偏移量, -1 使用文件默认偏移量
  int _file_offset = -1;
  WriteRequest(const int file_fd, char* data, const uint32_t data_size)
      : fd_(file_fd), data_(data), data_size_(data_size) {}
};

// 就绪事件组
struct ConSumptionQueue {
  struct ::io_uring_cqe* _con_queue_ = nullptr;
  auto getData() {
    return ::io_uring_cqe_get_data(_con_queue_);
  }
  bool isEmpty() {
    return _con_queue_ == nullptr;
  }
};

class IOUring : public NonCopyable {
 public:
  IOUring() : isInit_(false) {}
  ~IOUring();
  /*
    @ring_depth: io_uring 的队列长度
  */
  bool Init(const uint32_t ring_depth);
  /*
    停止一个 io_uring 实例
  */
  void Stop();

  /*
    @request: 准备读取请求, 一次读请求, 需要保证
    * 事件就绪之前, buf 的生命周期一直存在
    *
  */
  void PrepRead(ReadRequest* read_request);

  /*
    @request: 准备写请求, 一次写请求, 不保证
    *         读写的顺序性, 即 read & write 与顺序执行的结果不同.
    写请求
  */
  void PrepWrite(WriteRequest* write_request);



  // 将本次读取请求与下一次请求链接, 保证有序性
  void PrepReadLink(ReadRequest* read_request);

  // 将本次写入请求与下一次请求链接, 保证有序性
  void PrepWriteLink(WriteRequest* write_request);

  /*
    提交请求
  */
  uint32_t Submit();

  /*
   * no_block
   * 返回一组就绪事件
   * 不能忽略返回值
   */
  [[nodiscard]] struct ConSumptionQueue PeekFinishQueue();

  /*
   * block 等待就绪事件完成
   * 返回一组就绪事件
   * 不能忽略返回值
   */
  [[nodiscard]] struct ConSumptionQueue WaitFinishQueue();

  // 将已完成事件标记为 完成
  // 取消重复通知
  // @return : no_return
  // @struct ConSumptionQueue: que != nullptr
  // 该参数应该由 PeekFinishQueue and WaitFinishQueue 返回.
  void DeleteEvent(struct ConSumptionQueue* que);

 private:
  // io_uring 的队列长度
  uint32_t ring_depth_;
  // io_uring 句柄
  struct ::io_uring ring_;
  // 是否 init
  bool isInit_;
};

}  // namespace iouring

}  // namespace util

}  // namespace fver

#endif