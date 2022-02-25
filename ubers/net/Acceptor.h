#ifndef _UBERS_NET_ACCEPTOR_H_
#define _UBERS_NET_ACCEPTOR_H_
#include <functional>
#include "Channel.h"
#include "Socket.h"

namespace UBERS::net
{
class EventLoop;
class InetAddress;

class Acceptor : boost::noncopyable
{
public:
  using NewConnectionCallback = std::function<void (int sockfd, const InetAddress&)>;

  Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport);

  ~Acceptor();

  void SetNewConnectionCallback(const NewConnectionCallback& func);

  void Listen();
  bool IsListen() { return this->listening_;}

private:
  void HandleRead();

  EventLoop* loop_;

  Socket acceptsocket_;
  Channel acceptchannel_;
  NewConnectionCallback newconncallback_;

  bool listening_;
  int idleFd_;
};
}
#endif