#ifndef _UBERS_HTTPRESPONSE_H_
#define _UBERS_HTTPRESPONSE_H_

#include <boost/noncopyable.hpp>
#include <map>
#include <string>

namespace UBERS::net
{
class Buffer;
/*
HTTP/1.1 200 ok\r\n
Content-Length: 112\r\n
Connection: Keep-Alive\r\n
Content-Type: text/html\r\n
Server: Muduo\r\n

body

*/
class HttpResponse : public boost::noncopyable
{
public:
  enum HttpStatusCode
  {
    unknown = -1,
    ok = 200,
    no_content = 204,
    partial_content = 206,
    moved_permanently = 301,
    found = 302,
    see_other = 303,
    not_modified = 304,
    temporary_redirect = 307,
    bad_request = 400,
    unauthorized = 401,
    forbidden = 403,
    not_fount = 404,
    internel_server_error = 500,
    service_unavailable = 503,
  };
  explicit HttpResponse(bool close) : statuscode_(unknown), closeconnection_(close) {}

  void SetStatusCode(HttpStatusCode code) { this->statuscode_ = code;}

  void SetStatusMessage(const std::string& message) { this->statusmessage_ = message;}

  void SetCloseConnection(bool on) { closeconnection_ = on;}
  void SetContentType(const std::string& contenttype) { Addheader("Content-Type", contenttype);}

  void Addheader(const std::string& key, const std::string& value) { headers_[key] = value;}
  void SetBody(const std::string& body) { body_ = body;}
  void AppendToBuffer(Buffer* output) const;

private:
  std::map<std::string, std::string> headers_;
  HttpStatusCode statuscode_;

  std::string statusmessage_;

  bool closeconnection_;
  
  std::string body_;
};
}

#endif