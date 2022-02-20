#ifndef _UBERS_NET_TCPSERVER_H_
#define _UBERS_NET_TCPSERVER_H_
#include <map>
#include <memory>
#include <assert.h>
#include <functional>
#include <atomic>
#include <boost/noncopyable.hpp>
#include "TcpConnetion.h"
#include "Channel.h"
#include "CallBack.h"

namespace UBERS::net
{
class EventLoop;
class EventLoopThreadPool;

class TcpServer : public boost::noncopyable
{
public:
  using ThreadInitCallBack = std::function<void(EventLoop* )>;
  explicit TcpServer(EventLoop* loop, struct sockaddr_in ListenAddr);
  ~TcpServer();
  [[nodiscard]] EventLoop* GetLoop() { return this->loop_;}
  void SetThreadNum(int n );

  void Start();

  void SetThreadInitCallBack(ThreadInitCallBack func) { this->threadInit_callback_ = std::move(func);}
  void SetConnectionCallBack(ConnectionCallBack func) { this->connection_callback_ = std::move(func);}
  void SetWriteCompleteCallBack(WriteCompleteCallBack func) { this->writecomplete_callback_ = std::move(func);}
  void SetCloseCallBack(CloseCallBack func) { this->close_callback_ = std::move(func);}
  void SetMessageCallBack(MessageCallBack func) { this->message_callback_ = std::move(func);}

  void AcceptSocketListen();
private:
  void NewConnection(int Sockfd);
  void AcceptHandRead();

  EventLoop* loop_ = nullptr;
  std::unique_ptr<EventLoopThreadPool> threadpool_;
  
  ConnectionCallBack connection_callback_;
  CloseCallBack close_callback_;
  MessageCallBack message_callback_;
  WriteCompleteCallBack writecomplete_callback_;
  ThreadInitCallBack threadInit_callback_;

  std::atomic<bool> started_;
  int acceptFd_;
  int IdleFd_;
  Channel acceptChannel_;
};
}
#endif