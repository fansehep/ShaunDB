#include "File.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/stat.h>


using namespace UBERS;


WriteFile::WriteFile(std::string_view FileName)
          : fp_(fopen(FileName.data(), "ae"))
{
  int err = ferror(fp_);
  setbuffer(fp_, buffer_, sizeof(buffer_));
}

WriteFile::~WriteFile()
{
  flush();
  fclose(fp_);
}

void WriteFile::append(const char* log, const size_t len)
{
  size_t n = 0;
  size_t remain = len;
  do
  {
    size_t x = this->write(log + n, remain);
    if(x == 0)
    {
      int err = ferror(fp_);
      if(err)
      {
        fprintf(stderr, "WriteFile error!\n");
        break;
      }
      n += x; remain = len - n;
    }
  } while (remain > 0);
  WriteLen_ += len;
}

ReadSmallFile::ReadSmallFile(std::string_view fileName)
  : fd_(open(fileName.data(), O_RDONLY | O_CLOEXEC))
{
  buf_[0] = '\0';
  if(fd_ < 0)
  {
    err_ = errno;
  }
}

int ReadSmallFile::ReadToBuffer(ssize_t* size)
{
  int err = err_;
  if(fd_ > 0)
  {
    ssize_t n = pread(fd_, buf_, sizeof(buf_) - 1, 0);
    if(n >= 0)
    {
      if(size)
      {
        *size = static_cast<ssize_t>(n);
      }
      buf_[n] = '\0';
    }
    else
    {
      err = errno;
    }
  }
  return err;
}

SendFileUtil::SendFileUtil(std::string_view fileName) :
   Fd_(open(fileName.data(), O_RDONLY | O_CLOEXEC))
{
  if(Fd_ < 0)
  {
    err_ = errno;
  }
  else
  {
    struct stat statBuf{};
    fstat(Fd_, &statBuf);
    sendlen_ = static_cast<size_t>(statBuf.st_size);
  }
}

SendFileUtil::~SendFileUtil()
{
  if(Fd_ >= 0)
  {
    close(Fd_);
  }
}

