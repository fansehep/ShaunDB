#include "LogFile.h"

using namespace UBERS;


LogFile::LogFile(std::string_view basename, size_t RollSize = 1024 * 60, bool threadsafe = true)
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
  }
  {
  }
}
