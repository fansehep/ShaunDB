
#ifndef _UBERS_HTTP_HTTPSERVER_H_
#define _UBERS_HTTP_HTTPSERVER_H_
#include "../net/TcpServer.h"
#include "boost/noncopyable.hpp"
#include "../base/ThreadPool.h"
#include "../net/EventLoop.h"

namespace UBERS::net
{
class HttpRequest;
class HttpResponse;

class HttpServer : public boost::noncopyable
{
public:
  using HttpCallBack = std::function<void (const HttpRequest&, HttpResponse*)>;




private:
  void OnConnection(const TcpConnectionPtr& conn);
  void OnMessage(const TcpConnectionPtr& conn, Buffer* buf, TimeStamp ReceiveTime);
  void OnRequest(const TcpConnectionPtr&, const HttpRequest&);

  TcpServer server_;
  HttpCallBack httpcallback_;
};
}
#endif