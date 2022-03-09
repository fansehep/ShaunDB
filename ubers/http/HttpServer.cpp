#include "HttpServer.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "HttpContext.h"
#include "../base/Logging.h"

namespace UBERS::net::detail
{
void defaulthttpcallback(const HttpRequest&, HttpResponse* res)
{
  res->SetStatusCode(HttpResponse::not_fount);
  res->SetStatusMessage("404 not found");
  res->SetCloseConnection(true);
}
} // namespace UEBRS::net::detail::defaulthttpcallback

namespace UBERS::net
{

HttpServer::HttpServer(EventLoop* loop, struct sockaddr_in& listenaddr)
  : server_(loop, listenaddr),
    httpcallback_(detail::defaulthttpcallback)
{
  server_.setConnectionCallBack(std::bind(&HttpServer::OnConnection, this, std::placeholders::_1));
  server_.SetMessageCallBack(std::bind(&HttpServer::OnMessage, this, std::placeholders::_1, std::placeholders::_2));
}

void HttpServer::Start()
{
  LOG_WARN << "HttpServer[" << "] starts listening on ";
  server_.Start();
}

void HttpServer::OnMessage(const TcpConnectionPtr& conn, Buffer* buf)
{
  HttpContext* context = boost::any_cast<HttpContext>(conn->GetMutableContext());
  if(!context->ParseRequest(buf))
  {
    conn->Send("HTTP/1.1 400 Bad Request\r\n\r\n");
    conn->Shutdown();
  }
  if(context->GetAll())
  {
    OnRequest(conn, context->request());
    context->Reset();
  }
}

void HttpServer::OnRequest(const TcpConnectionPtr& conn, const HttpRequest& req)
{
  const std::string& connc = req.Getheader("Connection");
  bool close = connc == "close" || (req.Getversion() == HttpRequest::HTTP1_0 && connc != "Keep-Alive");
  HttpResponse response(close);
  httpcallback_(req, &response);
  Buffer buf;
  response.AppendToBuffer(&buf);
  
}

}