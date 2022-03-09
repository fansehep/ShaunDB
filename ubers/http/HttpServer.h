
#ifndef _UBERS_HTTP_HTTPSERVER_H_
#define _UBERS_HTTP_HTTPSERVER_H_
#include "../net/TcpServer.h"
#include "boost/noncopyable.hpp"
#include "../base/ThreadPool.h"
#include "../net/EventLoop.h"

namespace UBERS::net{

class HttpRequest;
class HttpResponse;

class HttpServer : public boost::noncopyable
{
  using HttpCallback = std::function<void (const HttpRequest&, HttpResponse*)>;
  // using std::placeholders;
  HttpServer(EventLoop* loop, struct sockaddr_in& listenaddr);

  EventLoop* GetLoop() { return server_.GetEventLoop();}

  void SetThreadNums(int n) { this->server_.SetThreadNums(n);}

  void Start();
private: 
  void OnConnection(const TcpConnectionPtr& conn);
  void OnMessage(const TcpConnectionPtr& conn, Buffer* buf);
  void OnRequest(const TcpConnectionPtr&, const HttpRequest&);

  TcpServer server_;
  HttpCallback httpcallback_;
};
}
#endif