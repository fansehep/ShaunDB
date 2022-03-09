#include "HttpContext.h"
#include "../net/Buffer.h"

namespace UBERS::net
{
bool HttpContext::ParseRequest(Buffer* buf)
{
  bool ok = true;
  bool hasmore = true;
  while(hasmore)
  {
    if(state_ == EXPECTREQUESTLINE)
    {
      const char* crlf = buf->findCRLF();
      if(crlf)
      {
        ok = ProcessRequestline(buf->peek(), crlf);
        if(ok)
        {
          buf->retrieveUntil(crlf + 2);
          //
          state_ = EXPECTHEADERS;
        }
        else
        {
          hasmore = false;
        }
      }
      else
      {
        hasmore = false;
      }
    }
    else if(state_ == EXPECTHEADERS)
    {
      const char* crlf = buf->findCRLF();
      if(crlf)
      {
        const char* colon = std::find(buf->peek(), crlf, ':');
        if(colon != crlf)
        {
          request_.Addheader(buf->peek(), colon, crlf);
        }
        else
        {
          state_ = GOTALL;
          hasmore = false;
        }
        buf->retrieveUntil(crlf + 2);
      }
      else
      {
        hasmore = false;
      }
    }
    else if(state_ == EXPECTBODY)
    {

    }
  }
  return ok;
}

bool HttpContext::ProcessRequestline(const char* begin, const char* end)
{
  bool succeed = false;
  const char* start = begin;
  const char* space = std::find(start, end, ' ');
  if(space != end && request_.SetMethod(start, space))
  {
    start = space + 1;
    space = std::find(start, end, ' ');
    if(space != end)
    {
      const char* question = std::find(start, space, '?');
      if(question != space)
      {
        request_.Setpath(start, question);
        request_.SetQuery(question, space);
      }
      else
      {
        request_.Setpath(start, space);
      }
      start = space + 1;
      succeed = end - start == 8 && std::equal(start, end - 1, "HTTP/1.");
      if(succeed)
      {
        if(*(end - 1) == '1')
        {
          request_.Setversion(HttpRequest::HTTP1_1);
        }
        else if(*(end - 1) == '0')
        {
          request_.Setversion(HttpRequest::HTTP1_0);
        }
        else
        {
          succeed = false;
        }
      }
    }
  }
  return succeed;
}

}