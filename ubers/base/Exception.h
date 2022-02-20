#ifndef _UBERS_EXCEPTION_H_
#define _UBERS_EXCEPTION_H_

#include <exception>
#include <string>
#include <utility>

namespace UBERS
{
//? 继承标准类可以写异常类
class Exception : public std::exception
{
  public:
    explicit Exception(const char* what) : message_(what) 
    {
      FillStackTrace();
    }
    explicit Exception(std::string what) : message_(std::move(what))
    {
      FillStackTrace();
    }
    ~Exception() noexcept override = default;
    const char* what() const noexcept override 
    {
      return message_.c_str();
    }
    const char* stackTrace() const noexcept 
    {
      return stack_.c_str();
    }
  private:
    void FillStackTrace();
    static std::string demangle(const char* symbol);
    //* 异常信息
    std::string message_;
    //* 异常发生时，保存发生异常的调用栈信息
    std::string stack_;
};
}
#endif