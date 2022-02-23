#ifndef _UBERS_ASYNCLOGTHREAD_H_
#define _UBERS_ASYNCLOGTHREAD_H_
#include <vector>
#include <string>
#include <functional>
#include <condition_variable>
#include <memory>
#include <boost/thread/latch.hpp>
#include "Thread.h"
#include "LogStream.h"

//* 异步日志线程的封装
namespace UBERS::base
{
class AsyncLogThread : public boost::noncopyable
{
public:
  explicit AsyncLogThread(std::string basename, int flushSecond = 3, size_t rollSize = 64 * 1024);
  ~AsyncLogThread()
  {
    if(running_)
    {
      stop();
    }
  }

  void append(const char* log_, size_t len);
  void start();
  void stop();

private:
  void ThreadFunc();
  using Buffer = detail::FixedBuffer<detail::kLargeBuffer>;
  using BufferVector = std::vector<std::unique_ptr<Buffer>>;
  using BufferPtr = std::unique_ptr<Buffer>;

  //* 日志刷新时间
  const int flushSecond_;
  const size_t rollSize_;
  bool running_;
  //* 日志名称
  std::string basename_;
  //* 异步日志线程
  Thread thread_;
  std::mutex mutex_;
  std::condition_variable cond_;

  BufferPtr currentBuffer_;
  BufferPtr nextBuffer_;
  BufferVector buffers_;


  boost::latch latch_;
};
}
#endif