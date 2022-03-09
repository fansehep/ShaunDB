#include "HttpResponse.h"
#include "../net/Buffer.h"
#include <stdio.h>

namespace UBERS::net
{
/*
HTTP/1.1 200 ok\r\n
Content-Length: 112\r\n
Connection: Keep-Alive\r\n
Content-Type: text/html\r\n
Server: Muduo\r\n

body

*/

void HttpResponse::AppendToBuffer(Buffer* output) const
{
  char buf[32];
  snprintf(buf, sizeof(buf), "HTTP/1.1 %d ", statuscode_);
  output->append(buf);
  output->append(statusmessage_);
  output->append("\r\n");

  if(closeconnection_)
  {
    output->append("Connection: close\r\n");
  }
  else
  {
    snprintf(buf, sizeof(buf), "Content-Length: %zd\r\n", body_.size());
    output->append(buf);
    output->append("Connection: Keep-Alive\r\n");
  }

  for(const auto& header : headers_)
  {
    output->append(header.first);
    output->append(": ");
    output->append(header.second);
    output->append("\r\n");
  }

  output->append("\r\n");
  output->append(body_);
}



}