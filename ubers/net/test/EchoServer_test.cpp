#include "../TcpServer.h"
#include "../../base/Logging.h"
#include "../EventLoop.h"

#include "../TcpConnetion.h"
#include <functional>

/*
g++ EchoServer_test.cpp ../Buffer.cpp ../Channel.cpp ../Epoll.cpp ../EventLoop.cpp ../EventLoopThread.cpp ../EventLoopThreadPool.cpp  ../TcpConnection.cpp ../TcpServer.cpp ../Timer.cpp ../TimerManager.cpp ../SocketOps.cpp  ../../base/AsyncLogThread.cpp ../../base/Exception.cpp ../../base/File.cpp ../../base/LogFile.cpp ../../base/Logging.cpp ../../base/LogStream.cpp ../../base/Thread.cpp ../../base/ThreadPool.cpp ../../base/ThreadSafeQueue.hpp ../../base/TimeStamp.cpp -o EchoServer -lpthread -lboost_thread -std=c++20
*/
using namespace UBERS;
using namespace UBERS::net;
using namespace std::placeholders;
class EchoServer
{
public:
  EchoServer(EventLoop* loop, const struct sockaddr_in& listenaddr) \
    : loop_(loop),
      server_(loop, listenaddr)
  {
    server_.setConnectionCallBack(std::bind(&EchoServer::onConnection, this, _1));
    server_.SetMessageCallBack(std::bind(&EchoServer::onMessage, this, _1, _2));
    server_.SetThreadNums(2);
  }

  void Start()
  {
    server_.Start();
  }
private:
  void onConnection(const TcpConnectionPtr& conn)
  {
    LOG_INFO << "onConnection";
    conn->Send("hello");
  }
  void onMessage(const TcpConnectionPtr& conn, Buffer* buf)
  {
    std::string msg(buf->retrieveAllAsString());
    LOG_INFO << "onMessage";
    if(msg == "exit\n")
    {
      conn->Send("bye\n");
      conn->Shutdown();
    }
    if(msg == "quit\n")
    {
      loop_->Quit();
    }
    conn->Send(msg);
  }

  EventLoop* loop_;
  TcpServer server_;

};

int main(int argc, char* argv[])
{

  LOG_INFO << "echo server ";
  EventLoop loop;
  struct sockaddr_in listenaddr;
  bzero(&listenaddr, sizeof(listenaddr));
  listenaddr.sin_port = sockets::HostToNetwork16(2002);
  listenaddr.sin_addr.s_addr = INADDR_ANY;
  listenaddr.sin_family = AF_INET;
  EchoServer server(&loop, listenaddr);

  server.Start();
  loop.Loop();

  return 0;
}