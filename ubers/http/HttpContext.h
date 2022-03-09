#ifndef _UBERS_HTTP_CONTEXT_H_
#define _UBERS_HTTP_CONTEXT_H_
#include "HttpContext.h"
#include <boost/noncopyable.hpp>
#include "HttpRequest.h"
namespace UBERS::net
{
class Buffer;

class HttpContext : public boost::noncopyable
{
public:
  enum HttpRequestParseState
  {
    //* 渴望受到处理请求的状态
    EXPECTREQUESTLINE,
    //* 渴望处理头部信息的状态 
    EXPECTHEADERS,
    //* 处于请求 body 的状态
    EXPECTBODY,
    //*
    GOTALL,
  };
  HttpContext() : state_(EXPECTREQUESTLINE) {} 

  bool ParseRequest(Buffer* buf);

  bool GetAll() const { return state_ == GOTALL;}
  void Reset()
  {
    state_ = EXPECTREQUESTLINE;
    HttpRequest dummy;
    request_.Swap(dummy);
  }

  const HttpRequest& request() const { return request_;}
  HttpRequest& request() { return request_;}

private:
 
  bool ProcessRequestline(const char* begin, const char* end);
  //* 当前请求状态
  HttpRequestParseState state_;
  //* Http 请求
  HttpRequest request_;
};
}
#endif