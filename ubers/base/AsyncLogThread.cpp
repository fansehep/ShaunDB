#include <memory>
#include "AsyncLogThread.h"
#include "LogFile.h"

using namespace UBERS;
using namespace UBERS::base;

AsyncLogThread::AsyncLogThread(std::string basename, int flushSecond, size_t rollSize)
  : flushSecond_(flushSecond),
    rollSize_(rollSize),
    running_(false),
    basename_(std::move(basename)),
    thread_([this] {ThreadFunc();}),
    mutex_(),
    cond_(),
    currentBuffer_(std::make_unique<Buffer>()),
    nextBuffer_(std::make_unique<Buffer>()),
    buffers_(),
    latch_(1)
{
  currentBuffer_->bzero();
  nextBuffer_->bzero();
  buffers_.reserve(16);
} 

void AsyncLogThread::start()
{
  running_ = true;
  thread_.start();
  latch_.wait();
}

void AsyncLogThread::stop()
{
  running_ = false;
  cond_.notify_one();
}

void AsyncLogThread::append(const char* log_, size_t len)
{
  std::unique_lock<std::mutex> lock(mutex_);
  if(currentBuffer_->avail() > static_cast<size_t>(len))
  {
    
  } 
}