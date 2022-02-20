#include <execinfo.h>
#include <cxxabi.h>
#include <stdlib.h>
#include "Exception.h"

using namespace UBERS;


void Exception::FillStackTrace()
{
  const int kSize = 100;
  void* buffer[kSize];
  //* 获取当前线程的函数调用的堆栈
  //* backtrace : 栈回溯，保存各个栈帧的地址
  int nptrs = ::backtrace(buffer, kSize);
  //* 根据地址，保存符号信息, 返回值实际上是一个指针数组
  char** strings = ::backtrace_symbols(buffer, nptrs);

  if(strings)
  {
    for(int i = 0; i < nptrs; ++i)
    {
      stack_.append(demangle(strings[i]));
      stack_.push_back('\n');
    }
    //* backtrace_symbols 的返回是一个指向堆区的数组，需要 free 
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
    //* 由于函数重载所引起的函数名称紊乱，我们将其还原。
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