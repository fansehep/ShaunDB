#ifndef _UBERS_TCPCONNECTION_
#define _UBERS_TCPCONNECTION_
#include "CallBack.h"
#include <atomic>
#include <string_view>
#include <memory>
#include <boost/noncopyable.hpp>
#include "SocketOps.h"
#include <boost/any.hpp>
#include "Buffer.h"

//* TcpServer 管理多个sharedptr<TcpConnection> 
namespace UBERS::net
{
class Channel;
class EventLoop;


class TcpConnection : public boost::noncopyable,
                      public std::enable_shared_from_this<TcpConnection>
{
public:
  TcpConnection(EventLoop* loop, int Sockfd);
  ~TcpConnection();
  EventLoop* Getloop() { return this->loop_;}

  void Send(std::string_view message);
  void Send(const char* message, size_t size);
  void Send(Buffer* message);
  void Shutdown();
  void ForceClose();
  void SetTcpNagle(bool ue);
  bool IsReading();
  void Startread();
  void Stopread();

  boost::any* GetMutableContext()
  {
    return &(this->context_);
  }

  Buffer* GetInputBuffer() { return &this->inputbuffer_;}
  Buffer* GetOutputBuffer() { return &this->outputbuffer_;}
  void SetConnectionCallBack(const ConnectionCallBack& func)
  {
    this->conncallback_ = func;
  }
  void SetMessageCallBack(const MessageCallBack& func)
  {
    this->messcallback_ = func;
  }
  void SetWriteCompleteCallBack(const WriteCompleteCallBack& func)
  {
    this->writecompcallback_ = func;
  }
  void SetCloseCallBack(const CloseCallBack& func)
  {
    this->closecallback_ = func;
  }

  void ConnectEstablished();
  void ConnectDestroyed();
  void ConnectReset(int fd);

private:

  enum State{
    kDisconnected,
    kConnecting,
    kConnected,
    kDisconnecting
  };

  void HandleRead();
  void HandleWrite();
  void HandleClose();
  void HandleError();

 // void SendInLoop(std::string_view mess);
  void SendInLoop(const void* data, size_t len);
  void ShutDownInLoop();
  void ForceCloseInLoop();
  void StartReadInLoop();
  void StopReadInLoop();
  

  EventLoop* loop_;
  std::atomic<uint8_t> state_;
  bool reading_;
  
  int socketfd_;
  std::unique_ptr<Channel> channel_;

  ConnectionCallBack conncallback_;
  MessageCallBack messcallback_;
  WriteCompleteCallBack writecompcallback_;
  CloseCallBack closecallback_;

  Buffer inputbuffer_;
  Buffer outputbuffer_;

  boost::any context_;
};
using TcpConnectionptr = std::shared_ptr<TcpConnection>;

}
#endif