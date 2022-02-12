#ifndef _UBERS_NET_BUFFER_H_
#define _UBERS_NET_BUFFER_H_
#include <string.h>
#include <vector>
#include <assert.h>
#include <string>
#include <algorithm>
namespace UBERS::net
{
class Buffer
{
public:
  constexpr static const size_t kPrepend = 8;
  constexpr static const size_t kInitSize = 1024;
  explicit Buffer(size_t initialSize = kInitSize)
    : buffer_(kPrepend + initialSize),
      readerIndex_(kPrepend),
      writerIndex_(kPrepend)
  {
  }
  ~Buffer() = default;

  void swap(Buffer& temp)
  {
    buffer_.swap(temp.buffer_);
    std::swap(readerIndex_, temp.readerIndex_);
    std::swap(writerIndex_, temp.writerIndex_);
  }

  [[nodiscard]] size_t ReadableBytes() const { return writerIndex_ - readerIndex_;}
  [[nodiscard]] size_t WriteableBytes() const { return buffer_.size() - writerIndex_;}
  [[nodiscard]] size_t PrependableBytes() const { return readerIndex_;}
  [[nodiscard]] const char* Peek() const { return begin() + writerIndex_;}
  char* BeginWrite() { return begin() + writerIndex_;}
  [[nodiscard]] const char* BeginWrite() const { return begin() + writerIndex_;}
  [[nodiscard]] const char* FindCRLF() const
  {
    const char* crlf = std::search(Peek(), BeginWrite(), kCRLF, kCRLF + 2);
    return crlf == BeginWrite() ? nullptr : crlf;
  }

  const char* FindCRLF(const char* start) const
  {
    assert(Peek() <= start);
    assert(start <= BeginWrite());
    const char* crlf = std::search(start, BeginWrite(), kCRLF, kCRLF + 2);
    return crlf == BeginWrite() ? nullptr : crlf;
  }

  [[nodiscard]] const char* findEOL() const
  {
    const void* eol = memchr(Peek(), '\n', ReadableBytes());
    return static_cast<const char*>(eol);
  }

  void Retrieve(size_t len)
  {
    assert(len <= ReadableBytes());
    if(len < readerIndex_)
    {
      readerIndex_ += len;
    }
    else
    {
      RetrieveAll();
    }
  }

  void RetrieveUntil(const char* end)
  {
    assert(Peek() <= end);
    assert(end <= BeginWrite());
    Retrieve(end - Peek());
  }

  void RetrieveInt64() { Retrieve(sizeof(int64_t));}
  void RetrieveInt32() { Retrieve(sizeof(int32_t));}
  void RetrieveInt16() { Retrieve(sizeof(int16_t));}
  void RetrieveInt8() { Retrieve(sizeof(int8_t));}

  void RetrieveAll()
  {
     readerIndex_ = kPrepend;
     writerIndex_ = kPrepend;
  }

  std::string RetrieveAllAsString() { return RetrieveAsString(ReadableBytes());}
  std::string RetrieveAsString(size_t len)
  {
    assert(len <= ReadableBytes());
    std::string Result(Peek(), len);
    Retrieve(len);
    return Result;
  }

  void Append(const std::string& str)
  {
    Append(str.data(), str.size());
  }

  void Append(const char* str)
  {
    Append(str, ::strlen(str));
  }

  //FIXME
  void Append(const char* data, size_t len)
  { 
    EnsureWriteableBytes(len);
    std::copy(data, data + len, BeginWrite());
    HasWritten(len);
  }

  void Append(const void* data, size_t len) 
  { 
    Append(static_cast<const char*>(data), len);
  }

  void HasWritten(size_t len)
  {
    assert(len <= WriteableBytes());
    writerIndex_ += len;
  }

  void UnWrite(size_t len)
  {
    assert(len <= WriteableBytes());
    writerIndex_ -= len; 
  }

  void AppendInt64(int64_t x)
  {
    int64_t be64 = htobe64(x);
    Append(&be64, sizeof(be64));
  }

  void AppendInt32(int32_t x)
  {
    int32_t be32 = htobe32(x);
    Append(&be32, sizeof(be32));
  }

  void AppendInt16(int16_t x)
  {
    int16_t be16 = htobe16(x);
    Append(&be16, sizeof(be16));
  }

  void AppendInt8(int8_t x)
  {
    Append(&x, sizeof(int8_t));
  }

  int64_t ReadInt64()
  {
    int64_t result = PeekInt64();
    RetrieveInt64();
    return result;
  }

  int32_t ReadInt32()
  {
    int32_t result = PeekInt32();
    RetrieveInt32();
    return result;
  }

  int16_t ReadInt16()
  {
    int16_t result = PeekInt16();
    RetrieveInt16();
    return result;
  }

  int8_t ReadInt8()
  {
    int8_t result = PeekInt8();
    RetrieveInt8();
    return result;
  }
  
  void PrependInt64(int64_t x)
  {
    int64_t be64 = htobe64(x);
    Append(&be64, sizeof(be64));
  }
  
  void PrependInt32(int32_t x)
  {
    int32_t be64 = htobe64(x);
    Append(&be64, sizeof(be64));
  }

  void PrependInt16(int16_t x)
  {
    int16_t be16 = htobe64(x);
    Append(&be16, sizeof(be16));
  }

  void PrependInt8(int8_t x)
  {
    Append(&x, sizeof(x));
  }

  void Prepend(const void* data, size_t len)
  {
    assert(len <= PrependableBytes());
    readerIndex_ -= len;
    const char* d = static_cast<const char*>(data);
    std::copy(d, d + len, begin() + readerIndex_);
  }

  void Shrink(size_t reserve)
  {
    EnsureWriteableBytes(ReadableBytes() + reserve);
    buffer_.resize(ReadableBytes() + reserve + kInitSize);
    buffer_.shrink_to_fit();
  }

  [[nodiscard]] size_t InternalCapacity() const { return buffer_.capacity();}
  size_t ReadFd(int fd, int* saveErrno);
private:
  char* begin() { return &*buffer_.begin();}
  [[nodiscard]] const char* begin() const { return &*buffer_.begin();}
  [[nodiscard]] int64_t PeekInt64() const
  {
    assert(ReadableBytes() >= sizeof(int32_t));
    int32_t be32 = 0;
    ::memcpy(&be32, Peek(), sizeof(be32));
    return be32toh(be32);
  }

  [[nodiscard]] int32_t PeekInt32() const
  {
    assert(ReadableBytes() >= sizeof(int32_t));
    int32_t be32 = 0;
    ::memcpy(&be32, Peek(), sizeof(be32));
    return be32toh(be32);
  }

  [[nodiscard]] int16_t PeekInt16() const
  {
    assert(ReadableBytes() >= sizeof(int16_t));
    int32_t be32 = 0;
    ::memcpy(&be32, Peek(), sizeof(be32));
    return be32toh(be32);
  }

  [[nodiscard]] int8_t PeekInt8() const
  {
    assert(ReadableBytes() >= sizeof(int8_t));
    int8_t x = *Peek();
    return x;
  }

  void EnsureWriteableBytes(size_t len)
  {
    if(WriteableBytes() < len)
    {
      MakeSpace(len);
    }
  }

  void MakeSpace(size_t len)
  {
    if(WriteableBytes() + PrependableBytes() < len + kPrepend)
    {
      buffer_.resize(writerIndex_ + len);
    }
    else
    {
      assert(kPrepend < readerIndex_);
      size_t readable = ReadableBytes();
      std::copy(begin() + readerIndex_, begin() + writerIndex_, begin() + kPrepend);
      readerIndex_ = kPrepend;
      writerIndex_ = readerIndex_ + readable;
      assert(readable == ReadableBytes());
    }
  }

  std::vector<char> buffer_;
  size_t readerIndex_;
  size_t writerIndex_;

  static const char kCRLF[];
};
}
#endif