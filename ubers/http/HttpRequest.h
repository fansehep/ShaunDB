#ifndef _UBERS_HTTPREQUEST_H_
#define _UBERS_HTTPREQUEST_H_
#include <map>
#include <assert.h>
#include <string>
#include <string.h>
#include "../base/Logging.h"
#include "../base/TimeStamp.h"
#include <boost/lexical_cast.hpp>

/*
GET / HTTP /1.1\r\n
Accept: image/jpeg, application/x-ms-applition\r\n
Accept-Language: zh-CN\r\n
User-Agent: Mozilla/4.0\r\n
Host: 192.168.159.188.8080\r\n
Connection: Keep-Alive
*/
namespace UBERS::net
{
//* Http 请求的封装
class HttpRequest
{
public:
  enum Method
  {
    INVALID,
    GET,
    POST,
    PUT,
    HEAD,
    DELETE,
    OPTIONS,
    TRACE,
    CONNECT
  };

  enum Version
  {
    UNKOWN,
    HTTP1_0, 
    HTTP1_1
  };

  explicit HttpRequest()
    : method_(INVALID),
      version_(UNKOWN)
  { }


  bool SetMethod(const char* start, const char* end)
  {
    assert(method_ == INVALID);
    std::string m(start, end);
    if (m == "GET")
    {
      method_ = GET;
    }
    else if (m == "POST")
    {
      method_ = POST;
    }
    else if (m == "HEAD")
    {
      method_ = HEAD;
    }
    else if (m == "PUT")
    {
      method_ = PUT;
    }
    else if (m == "DELETE")
    {
      method_ = DELETE;
    }
    else
    {
      method_ = INVALID;
    }
    return method_ != INVALID;
  }
  Method method() const { return method_;}
  const char* methodString() const
  {
    const char* result = "UNKOWN";
    switch(method_)
    {
      case GET : result = "get"; break;
      case POST : result = "POST"; break;
      case HEAD : result = "HEAD"; break;
      case PUT : result = "PUT"; break;
      case DELETE : result = "DELETE"; break;
      case OPTIONS: result = "OPTIONS"; break;
      case CONNECT : result = "CONNECT"; break;
      case TRACE : result = "TRACE"; break;
      default : break;
    }
    return result;
  }
  void Setpath(const char* start, const char* end)
  { path_.assign(start, end);}

  void SetQuery(const char* start, const char* end)
  { query_.assign(start, end); }
 
  const std::string& query() const { return query_;}
  
  void SetReceiveTime(TimeStamp time) { this->receivetime_ = time;}
  auto GetReceiveTime() const { return receivetime_;}

  void Addheader(const char* start, const char* colon, const char* end)
  {
    std::string filed(start, colon);
    ++colon;
    while(colon < end && isspace(*colon))
    {
      ++colon;
    }
    std::string value(colon, end);
    while(!value.empty() && isspace(value[value.size() - 1]))
    {
      value.resize(value.size() - 1);
    }
    headers_[filed] = value;
  }

  std::string Getheader(const std::string& field) const
  {
    std::string result;
    std::map<std::string, std::string>::const_iterator it = headers_.find(field);
    if(it != headers_.end())
    {
      result = it->second;
    }
    return result;
  }

  const std::map<std::string, std::string>& headers() const { return this->headers_;}
  void Swap(HttpRequest& that)
  {
    std::swap(method_, that.method_);
    std::swap(version_, that.version_);
    path_.swap(that.path_);
    query_.swap(that.query_);
    receivetime_.swap(that.receivetime_);
    headers_.swap(that.headers_);
  }

  void Setversion(Version v) { this->version_ = v;}
  auto Getversion() {return this->version;}
private:
  //* 请求方法
  Method method_;
  //* HTTP 协议版本
  uint8_t version_;
  //* 请求路径
  std::string path_;
  //*
  std::string query_;
  //* 
  std::string body_;
 
  TimeStamp receivetime_;
 
  std::map<std::string, std::string> headers_;
};
}
#endif