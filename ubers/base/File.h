#ifndef _UBERS_FILE_H_
#define _UBERS_FILE_H_

#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <string>
#include <boost/noncopyable.hpp>


namespace UBERS
{
//*默认文件缓冲区大小
static const int kFileBufferSize = 64 * 1024;

class WriteFile : public boost::noncopyable
{
public:
  explicit WriteFile(std::string_view FileName);
  ~WriteFile();

  void append(const char* log, size_t len);
  void flush() { ::fflush(fp_); }
  size_t WriteLen() const { return this->WriteLen_;}
private:
  size_t write(const char* log, size_t len );
  FILE* fp_;
  size_t WriteLen_ = 0;
  char buffer_[kFileBufferSize];
};
//* 小文件 <= 64kb
class ReadSmallFile : public boost::noncopyable
{
public:
  explicit ReadSmallFile(std::string_view fileName);
  ~ReadSmallFile();

  int ReadToBuffer(ssize_t* size);
  const char* buffer() const { return buf_; }
private:
  int fd_;
  int err_ = 0;
  char buf_[kFileBufferSize];
};

class SendFileUtil
{
public:
  explicit SendFileUtil(std::string_view FileName);
  explicit SendFileUtil() {}
  ~SendFileUtil();

  int GetFd() const { return Fd_;}
  off_t* GetOffSet() { return &offset_;}
  size_t GetSendLen() const { return this->sendlen_;}
private:
  int Fd_ = -1;
  off_t offset_ = 0;
  size_t sendlen_ = 0;
  int err_ = 0;
};


}
#endif