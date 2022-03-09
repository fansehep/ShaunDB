#ifndef _UBERS_BUFFER_H_
#define _UBERS_BUFFER_H_
#include <cstring>
#include <vector>
#include <cassert>
#include <algorithm>
#include <string>
namespace UBERS::net
{

using std::string;
class Buffer
{
public:
    static const size_t kPrepend = 8;
    static const size_t kInitSize = 1024;
    explicit Buffer(size_t initialSize = kInitSize)
        : buffer_(kPrepend + initialSize),
          readerIndex_(kPrepend),
          writerIndex_(kPrepend)
    {
    }

    void swap(Buffer &rhs)
    {
        buffer_.swap(rhs.buffer_);
        std::swap(readerIndex_, rhs.readerIndex_);
        std::swap(writerIndex_, rhs.writerIndex_);
    }

    [[nodiscard]] size_t readableBytes() const { return writerIndex_ - readerIndex_; }
    [[nodiscard]] size_t writeableBytes() const { return buffer_.size() - writerIndex_; }
    [[nodiscard]] size_t prependableBytes() const { return readerIndex_; };
    [[nodiscard]] const char *peek() const { return begin() + readerIndex_; }
    char *beginWrite() { return begin() + writerIndex_; }
    [[nodiscard]] const char *beginWrite() const { return begin() + writerIndex_; }
    [[nodiscard]] const char *findCRLF() const
    {
        const char *crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF + 2);
        return crlf == beginWrite() ? nullptr : crlf;
    }

    const char *findCRLF(const char *start) const //从start处往后寻找结束符
    {
        assert(peek() <= start);
        assert(start <= beginWrite());
        const char *crlf = std::search(start, beginWrite(), kCRLF, kCRLF + 2);
        return crlf == beginWrite() ? nullptr : crlf;
    }

    [[nodiscard]] const char *findEOL() const //返回结束处
    {
        const void *eol = memchr(peek(), '\n', readableBytes());
        return static_cast<const char *>(eol);
    }

    const char *findEOL(const char *start) const //从start处返回结束处
    {
        assert(peek() <= start);
        assert(start <= beginWrite());
        const void *eol = memchr(start, '\n', beginWrite() - start);
        return static_cast<const char *>(eol);
    }

    void retrieve(size_t len)
    {
        assert(len <= readableBytes());
        if (len < readableBytes())
            readerIndex_ += len;
        else
            retrieveAll();
    }

    void retrieveUntil(const char *end)
    {
        assert(peek() <= end);
        assert(end <= beginWrite());
        retrieve(end - peek());
    }

    void retrieveInt64() { retrieve(sizeof(int64_t)); } //取回8字节
    void retrieveInt32() { retrieve(sizeof(int32_t)); } //4
    void retrieveInt16() { retrieve(sizeof(int16_t)); } //2
    void retrieveInt8() { retrieve(sizeof(int8_t)); }   //1
    void retrieveAll()                                  //取回所有，返回初始位置
    {
        readerIndex_ = kPrepend;
        writerIndex_ = kPrepend;
    }

    string retrieveAllAsString() { return retrieveAsString(readableBytes()); }
    string retrieveAsString(size_t len) //从可读处取len长度的字符串
    {
        assert(len <= readableBytes());
        string result(peek(), len); //peek返回的是可读端首
        retrieve(len);              //偏移到len处
        return result;
    }

    void append(const string &str) { append(str.data(), str.size()); }
    void append(const char *str) { append(str, strlen(str)); }

    void append(const char *data, size_t len) //添加数据
    {
        ensureWritableBytes(len); //如果可用空间不足len，则扩充缓冲区位置到len一遍可以完全放下数据
        std::copy(data, data + len, beginWrite());
        hasWritten(len);
    }

    void append(const void *data, size_t len) { append(static_cast<const char *>(data), len); }

    void hasWritten(size_t len)
    {
        assert(len <= writeableBytes());
        writerIndex_ += len;
    }

    void unwrite(size_t len)
    {
        assert(len <= writeableBytes());
        writerIndex_ -= len;
    }

    void appendInt64(int64_t x) //填充64位整数
    {
        int64_t be64 = htobe64(x);
        append(&be64, sizeof(be64));
    }

    void appendInt32(int32_t x) //填充32位整数
    {
        int32_t be32 = htobe32(x);
        append(&be32, sizeof(be32));
    }

    void appendInt16(int16_t x) //填充16位
    {
        int16_t be16 = htobe16(x);
        append(&be16, sizeof(be16));
    }

    void appendInt8(int8_t x) //填充8位
    {
        append(&x, sizeof(x));
    }

    int64_t readInt64() //读取64位
    {
        int64_t result = peekInt64();
        retrieveInt64(); //读走了所以位置调整
        return result;
    }

    int32_t readInt32() //读取32位
    {
        int32_t result = peekInt32();
        retrieveInt32(); //读走了所以位置调整
        return result;
    }

    int16_t readInt16()
    {
        int16_t result = peekInt16();
        retrieveInt16();
        return result;
    }

    int8_t readInt8()
    {
        int8_t result = peekInt8();
        retrieveInt8();
        return result;
    }

    void prependInt64(int64_t x) //在前面预留空间添加8个字节
    {
        int64_t be64 = htobe64(x); //转网络
        prepend(&be64, sizeof be64);
    }

    void prependInt32(int32_t x)
    {
        int32_t be32 = htobe32(x);
        prepend(&be32, sizeof be32);
    }

    void prependInt16(int16_t x)
    {
        int16_t be16 = htobe16(x);
        prepend(&be16, sizeof be16);
    }

    void prependInt8(int8_t x)
    {
        prepend(&x, sizeof x);
    }

    void prepend(const void *data, size_t len) //通过这个函数来时间想前面预留空间加
    {
        assert(len <= prependableBytes());
        readerIndex_ -= len;
        const char *d = static_cast<const char *>(data);
        std::copy(d, d + len, begin() + readerIndex_);
    }

    void shrink(size_t reserve) //伸缩空间，保留reserve个字节
    {
        ensureWritableBytes(readableBytes() + reserve);
        buffer_.resize(readableBytes() + reserve + kInitSize);
        buffer_.shrink_to_fit();
    }

    [[nodiscard]] size_t internalCapacity() const { return buffer_.capacity(); }
    ssize_t readFd(int fd, int *saveErrno);

    ~Buffer() = default;

private:
    char *begin() { return &*buffer_.begin(); }
    [[nodiscard]] const char *begin() const { return &*buffer_.begin(); }

    [[nodiscard]] int64_t peekInt64() const
    {
        assert(readableBytes() >= sizeof(int64_t));
        int64_t be64 = 0;
        ::memcpy(&be64, peek(), sizeof be64); //拷贝大小为64位的的到be64
        return be64toh(be64);                 //转为主机字节序
    }

    [[nodiscard]] int32_t peekInt32() const
    {
        assert(readableBytes() >= sizeof(int32_t));
        int32_t be32 = 0;
        ::memcpy(&be32, peek(), sizeof be32);
        return be32toh(be32);
    }

    [[nodiscard]] int16_t peekInt16() const
    {
        assert(readableBytes() >= sizeof(int16_t));
        int16_t be16 = 0;
        ::memcpy(&be16, peek(), sizeof be16);
        return be16toh(be16);
    }

    [[nodiscard]] int8_t peekInt8() const
    {
        assert(readableBytes() >= sizeof(int8_t));
        int8_t x = *peek();
        return x;
    }

    void ensureWritableBytes(size_t len)
    {
        if (writeableBytes() < len)
            makeSpace(len);
    }

    void makeSpace(size_t len) //扩充空间
    {
        if (writeableBytes() + prependableBytes() < len + kPrepend) //判断扩充空间
        {
            buffer_.resize(writerIndex_ + len); //如果缓存区不够用，使用resize扩容
        }
        else
        {
            assert(kPrepend < readerIndex_); //够用，进行移位存储下来
            size_t readable = readableBytes();
            std::copy(begin() + readerIndex_,
                      begin() + writerIndex_,
                      begin() + kPrepend);
            readerIndex_ = kPrepend;
            writerIndex_ = readerIndex_ + readable;
            assert(readable == readableBytes());
        }
    }

private:
    std::vector<char> buffer_; //vector用于代替固定大小的数组
    size_t readerIndex_;       //读位置
    size_t writerIndex_;       //写位置

    static const char kCRLF[]; //"\r\n"
};

} // namespace ssxrver::net
#endif
