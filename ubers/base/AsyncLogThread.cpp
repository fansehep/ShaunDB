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
  thread_.Stop();
}

void AsyncLogThread::append(const char* log_, size_t len)
{
  std::unique_lock<std::mutex> lock(mutex_);
  //* 如果当前 buffer的长度 > 需要记录的日志的长度
  if(currentBuffer_->avail() > static_cast<size_t>(len))
  {
    currentBuffer_->append(log_, len);
  } 
  else
  {
    buffers_.push_back(std::move(currentBuffer_));
    if(nextBuffer_)
    {
      currentBuffer_ = std::move(nextBuffer_);
    }
    else
    {
      currentBuffer_ = std::make_unique<Buffer>();
    }
    currentBuffer_->append(log_, len);
    cond_.notify_one();
  }
}

void AsyncLogThread::ThreadFunc()
{
  //* 双缓冲区日志的实现
  assert(running_);
  latch_.count_down();
  file::LogFile output(basename_, rollSize_);
  
  BufferPtr newBuffer_1 = std::make_unique<Buffer>();
  BufferPtr newBuffer_2 = std::make_unique<Buffer>();

  newBuffer_1->bzero();
  newBuffer_2->bzero();

  BufferVector buffersTowrite{};
  buffersTowrite.reserve(16);

  while(running_)
  {
    assert(newBuffer_1 && newBuffer_1->length() == 0);
    assert(newBuffer_2 && newBuffer_2->length() == 0);
    assert(buffersTowrite.empty());
    {
      std::unique_lock<std::mutex> lock(mutex_);
      //* 如果没有写入 或者 超时 
      if(buffers_.empty())
      {
        cond_.wait_for(lock, std::chrono::seconds(flushSecond_));
      }
      //* 无论是否写满，都写入buffers_中
      buffers_.push_back(std::move(currentBuffer_));
      //* newBuffer_1当作缓冲区
      currentBuffer_ = std::move(newBuffer_1);
      //* 前端日志 和 后端日志 交换 
      buffersTowrite.swap(buffers_);

      if(!nextBuffer_)
      {
        nextBuffer_ = std::move(newBuffer_2);
      }
    }
  assert(!buffersTowrite.empty());
  
  //* 数据太多，直接删除
  if(buffersTowrite.size() > 25)
  {
    buffersTowrite.erase(buffersTowrite.begin() + 2, buffersTowrite.end());
  }
  //* 开始写入
  for(const auto& buffer : buffersTowrite)
  {
    output.append(buffer->data(), buffer->length());
  }

  if(buffersTowrite.size() > 2)
  {
    buffersTowrite.resize(2);
  }

  if(!newBuffer_1)
  {
    assert(!buffersTowrite.empty());
    newBuffer_1 = std::move(buffersTowrite.back());
    buffersTowrite.pop_back();
    newBuffer_1->reset();
  }
  if(!newBuffer_2)
  {
    assert(!buffersTowrite.empty());
    newBuffer_2 = std::move(buffersTowrite.back());
    buffersTowrite.pop_back();
    newBuffer_2->reset();
  }

  buffersTowrite.clear();
  output.flush();
}
  output.flush();
}