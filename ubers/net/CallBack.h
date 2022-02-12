#ifndef _UBERS_NET_CALLBACK_H_
#define _UBERS_NET_CALLBACK_H_
#include <functional>
#include <memory>
namespace UBERS::net
{
  class Buffer;
  class TcpConnection;

  using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

  using ConnectionCallBack = std::function<void (const TcpConnectionPtr&)>;
  using CloseCallBack = std::function<void (const TcpConnectionPtr&)>;
  using WriteCompleteCallBack = std::function<void (const TcpConnectionPtr&)>;

  using TimerCallBack = std::function<void()>;
  using MessageCallBack = std::function<void (const TcpConnectionPtr&, Buffer*)>;

  void DefaultConnectionCallBack(const TcpConnectionPtr& conn);
  void DefaultMessageCallBack(const TcpConnectionPtr& conn, Buffer* buffer);
}
#endif