#ifndef _UBERS_NET_TCPSERVER_H_
#define _UBERS_NET_TCPSERVER_H_
#include <atomic>
#include "TcpConnetion.h"
#include <set>
#include "Channel.h"

namespace UBERS::net
{

class EventLoop;
class EventLoopThreadPool;

class TcpServer : public boost::noncopyable
{
public:
  using ThreadInitCallBack = std::function<void( EventLoop*)>;

  explicit TcpServer(EventLoop* loop, struct sockaddr_in listenaddr);
  ~TcpServer();

  void SetThreadNums(int n);

  void Start();
  EventLoop* GetEventLoop() { return loop_;}


  void SetThreadInitCallBack(ThreadInitCallBack func)
  {
    threadinitcallback_ = func;
  }
  void SetWriteCallBack(WriteCompleteCallBack func)
  {
    writecomplecallback_ = func;
  }
  void setConnectionCallBack(ConnectionCallBack func)
  {
    conncallback_ = func;
  }
  void SetMessageCallBack(MessageCallBack func)
  {
    messagecallback_ = func;
  }
  void acceptsocketlisten();
private:

  void NewConnection(int sockfd);
  void AcceptHandleRead();



  int idlefd_;
  int acceptfd_;
  Channel acceptchannel_;
  EventLoop* loop_;
  std::unique_ptr<EventLoopThreadPool> threadpool_;

  ConnectionCallBack conncallback_;
  MessageCallBack messagecallback_;
  WriteCompleteCallBack writecomplecallback_;
  ThreadInitCallBack threadinitcallback_;

  std::atomic<bool> started_;




};
}
#endif