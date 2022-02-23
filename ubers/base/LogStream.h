#ifndef _UBERS_LOGSTREAM_H_
#define _UBERS_LOGSTREAM_H_


#include <assert.h>
#include <string.h>
#include <string>
#include <boost/noncopyable.hpp>


namespace UBERS
{
namespace detail
{
  const int kSmallBuffer = 4000;
  const int kLargeBuffer = 4000 * 1000;
//* 日志缓冲区
template <int SIZE>
class FixedBuffer : public boost::noncopyable
{
public:
  explicit FixedBuffer() : cur_(data_) {}
  ~FixedBuffer() = default;
  //* 增加字符串
  void append(const char* buf, size_t len)
  {
    if(avail() > len)
    {
      memcpy(cur_, buf, len);
      cur_ += len;
    }
    else
    {
      if(avail() > 0)
      {
        memcpy(cur_, buf, static_cast<size_t>(avail()));
        cur_ += avail();
      }
    }
  }

  const char* data() const
  {
    return data_;
  }

  size_t length() const
  {
    return static_cast<size_t>(cur_ - data_);
  }

  void add(size_t len)
  {
    cur_ += len;
  }

  char* current()
  {
    return cur_;
  }
  //* 重置
  void reset()
  {
    cur_ = data_;
  }
  //* 返回可用空间，大小
  size_t avail() const
  {
    return static_cast<size_t>(end() - cur_);
  }
  //
  void bzero()
  {
    ::bzero(data_, sizeof(data_));
  }

private:
  const char* end() const { return data_ + sizeof(data_); }
  //* 缓冲区
  char data_[SIZE];
  //* 指向缓冲区当前位置
  char* cur_;
};
} // namespace UBERS::detail

class LogStream : public boost::noncopyable
{
private:
  using self = LogStream;
public:
  using Buffer = detail::FixedBuffer<detail::kSmallBuffer>;

  self& operator<<(bool s)
  {
    buffer_.append(s ? "1" : "0", 1);
    return *this;
  }
  //* 日志重载 << 
  self& operator << (short);
  self& operator << (unsigned short);
  self& operator << (int);
  self& operator << (unsigned int);
  self& operator << (long);
  self& operator << (unsigned long);
  self& operator << (long long);
  self& operator << (unsigned long long);
  self& operator << (const void*);
  self& operator << (double);


  self& operator << (float v)
  {
    *this << static_cast<double>(v);
    return *this;
  }

  self& operator << (char v)
  {
    buffer_.append(&v, 1);
    return *this;
  }

  self& operator << (const char* str)
  {
    if(str)
    {
      buffer_.append(str, strlen(str));
    }
    else
    {
      buffer_.append("(nullptr)", 8);
    }
    return *this;
  }

  self& operator << (const unsigned char* str)
  {
    return operator << (reinterpret_cast<const char*>(str));
  }

  self& operator << (const std::string& v)
  {
    buffer_.append(v.c_str(), v.size());
    return *this;
  }

  void append(const char* data, size_t len)
  {
    buffer_.append(data, len);
  }  

  const Buffer& buffer() const
  {
    return this->buffer_;
  }

  void resetBuffer()
  {
    buffer_.reset();
  }

private:
  Buffer buffer_;

  template<typename Type>
  void formatInteger(Type);

  static const int kMaxNumericSize = 21;
};


}
#endif