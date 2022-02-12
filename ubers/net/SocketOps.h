#ifndef _UBERS_NET_SOCKETOPS_H_
#define _UBERS_NET_SOCKETOPS_H_
#include <arpa/inet.h>


//* 对TCP网络系统调用的一些封装
namespace UBERS::net::socketops
{

//* 创建一个 TCP流式 非阻塞套接字,创建失败即终止
int CreateStreamNonBlockOrDie();  
//* 创建一个 UDP报文 非阻塞套接字，创建失败即终止
int CreateDgramNonBlockingOrDie();
//* 连接
int Connect(int ClientSockfd, struct sockaddr_in* addr);
//* 绑定
void BindOrDie(int ClientSockfd, struct sockaddr_in* addr);
//* 监听
void ListenOrDie(int SockFd);
//* 接受连接
int Accept(int Sockfd, struct sockaddr_in* addr);
//* 读
ssize_t Read(int Sockfd, void* buf, size_t count);
//* 读，以iovec结构体读取
ssize_t Readv(int Sockfd, const struct iovec* iov, int iovcnt);
//* 写
ssize_t Write(int Sockfd, const void* buf, size_t count);
//* 接受消息
ssize_t RecvFrom(int Sockfd, void* buf, size_t count, struct sockaddr_in* addr);
//* 发送消息
ssize_t Sendto(int Sockfd, const void* buf, size_t count, struct sockaddr_in* addr);
//* 发送文件
ssize_t SendFile(int Outfd, int Intfd, off_t* offset, size_t count);
//* 关闭文件描述符
void Close(int Sockfd);
//* 只关闭写
void ShutDownWrite(int Sockfd);
//* 心跳
void SetKeepAlive(int Sockfd, bool ue);
//* 是否开启 Negle 算法
void SetTcpNoDelay(int Sockfd, bool ue);
//* 地址复用
void SetReuseAddr(int Sockfd, bool ue);
//* 端口复用
void SetReusePort(int Sockfd, bool ue);
//* 获取文件描述符错误
int GetSocketError(int Sockfd);

//* 获取本地协议地址
struct sockaddr_in GetLocalAddr(int Sockfd);
//* 获取相连接的对方协议地址
struct sockaddr_in GetPeeraddr(int Sockfd);

}
#endif