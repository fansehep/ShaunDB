#include <execinfo.h>
#include <cxxabi.h>
#include <stdlib.h>
#include "Exception.h"

using namespace UBERS;


void Exception::FillStackTrace()
{
  enum { kSize = 100 };
  void* buffer[kSize];
  //* 获取当前线程的函数调用的堆栈
  int nptrs = ::backtrace(buffer, kSize);
  char** strings = ::backtrace_symbols(buffer, nptrs);

  if(strings)
  {
    for(int i = 0; i < nptrs; ++i)
    {
      stack_.append(demangle(strings[i]));
      stack_.push_back('\n');
    }
    free(strings);
  }
}

std::string Exception::demangle(const char* symbol)
{
  const int kSize = 128;
  size_t size;
  int status;
  char temp[kSize];
  char* demangled;
  if(1 == sscanf(symbol, "%*[^(]%*[^_]%127[^)+]", temp))
  {
    if(nullptr != (demangled = abi::__cxa_demangle(temp, nullptr, &size, &status)))
    {
      std::string result(demangled);
      free(demangled);
      return result;
    }
  }

  if(1 == sscanf(symbol, "%127s", temp))
  {
    return temp;
  }
  return symbol;
}