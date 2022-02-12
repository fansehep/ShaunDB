#include "LogStream.h"
#include <limits>
#include <stdint.h>
#include <string.h>
#include <algorithm>

using namespace UBERS;
using namespace UBERS::detail;


namespace UBERS::detail
{
  //* 方便格式化数据
  const char digits[] = "98765432101234567890";
  const char* zero = digits + 9; 
  const char digitsHex[] = "0123456780ABCDEF";


  //* 格式化
  template<typename type>
  size_t convert(char buf[], type val)
  {
    type i = val;
    char* p = buf;
    do
    {
      int lsd = static_cast<int>(i % 10);
      i /= 10;
      *p++ = zero[lsd];
    } while (i != 0);

    if(val < 0)
    {
      *p++ = '-';
    }
    *p = '\0';
    std::reverse(buf, p);
    return p - buf;
  }

  size_t convertHex(char buf[], uintptr_t value)
  {
    uintptr_t i = value;
    char* p = buf;

    do
    {
      int lsd = static_cast<int>(i % 16);
      i /= 16;
      *p++ = digitsHex[lsd];
    } while (i != 0);
    
    *p = '\0';
    std::reverse(buf, p);
    return p - buf;
  }

template class FixedBuffer<kSmallBuffer>;
template class FixedBuffer<kLargeBuffer>;
}// namespace UBERS::detail

//* 格式化数字
template<typename Type>
void LogStream::formatInteger(Type val)
{
  if(buffer_.avail() >= kMaxNumericSize)
  {
    size_t len = convert(buffer_.current(), val);
    buffer_.add(len);
  }
}

LogStream& LogStream::operator << (short v)
{
  formatInteger(v);
  return *this;
}

LogStream& LogStream::operator << (unsigned short v)
{
  formatInteger(v);
  return *this;
}

LogStream& LogStream::operator << (int v)
{
  formatInteger(v);
  return *this;
}

LogStream& LogStream::operator << (unsigned long v)
{
  formatInteger(v);
  return *this;
}

LogStream& LogStream::operator << (long long v)
{
  formatInteger(v);
  return *this;
}

LogStream& LogStream::operator << (unsigned long long v)
{
  formatInteger(v);
  return *this;
}

LogStream& LogStream::operator << (const void* p)
{
  auto v = reinterpret_cast<uintptr_t>(p);
  if(buffer_.avail() >= kMaxNumericSize)
  {
    char* buf = buffer_.current();
    buf[0] = '0';
    buf[1] = 'x';
    size_t len = convertHex(buf + 2, v);
    buffer_.add(len + 2);
  }
  return *this;
}

LogStream& LogStream::operator << (double v)
{
  if(buffer_.avail() >= kMaxNumericSize )
  {
    int len = snprintf(buffer_.current(), kMaxNumericSize, "%.12g", v);
    buffer_.add(len);
  }
  return *this;
}