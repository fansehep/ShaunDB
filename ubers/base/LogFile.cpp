#include "LogFile.h"
#include <time.h>
#include <memory>

using namespace UBERS;
using namespace UBERS::base::file;

LogFile::LogFile(std::string_view basename, size_t RollSize, bool threadsafe)
        : BaseName_(basename), RollSize_(RollSize), mutex_(threadsafe ? new std::mutex : nullptr)
{
  RollFile();
}

void LogFile::append(const char* logline, int len)
{
  if(mutex_)
  {
    std::lock_guard<std::mutex> mtx(*mutex_);
    append_unlocked(logline, len);
  }
  else
  {
    append_unlocked(logline, len);
  }
}

void LogFile::flush()
{
  if(mutex_)
  {
    std::lock_guard<std::mutex> mtx(*mutex_);
    file_->flush();
  }
  {
    file_->flush();
  }
}


void LogFile::RollFile()
{
  std::string fileName;
  GetLogFileName(fileName);
  file_ = std::make_unique<WriteFile>(fileName);
}


void LogFile::GetLogFileName(std::string& name)
{
  time_t now = 0;
  struct tm tm{};
  now = time(nullptr);
  gmtime_r(&now, &tm);
  char timeBUf[32];
  strftime(timeBUf, sizeof(timeBUf), ".%Y%m%d-%H%M%S.", &tm);
  name = BaseName_ + timeBUf;
  name += "log";
}

void LogFile::append_unlocked(const char* log_, size_t len)
{
  file_->append(log_, len);

  //* 可用空间
  if(file_->WriteLen() >= RollSize_)
  {
    RollFile();
  }
}

