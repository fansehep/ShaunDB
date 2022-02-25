#ifndef _UBERS_NET_SOCKETOPS_H_
#define _UBERS_NET_SOCKETOPS_H_
#include <arpa/inet.h>


//* 对TCP网络系统调用的一些封装
namespace UBERS::net::sockets
{
int CreateSocket(sa_family_t faml);
//* 创建一个 TCP流式 非阻塞套接字,创建失败即终止
int CreateStreamNonBlockOrDie();  
//* 创建一个 UDP报文 非阻塞套接字，创建失败即终止
int CreateDgramNonBlockingOrDie();
//* 连接
int Connect(int ClientSockfd, struct sockaddr* addr);
//* 绑定
void BindOrDie(int ClientSockfd, struct sockaddr* addr);
//* 监听
void ListenOrDie(int SockFd);
//* 接受连接
int Accept(int Sockfd, struct sockaddr_in6* addr);
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
//* 是否开启 Nagle 算法
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
struct sockaddr_in GetPeerAddr(int Sockfd);


//* 网络字节序和主机字节序的转换
inline uint64_t HostToNetwork64(uint64_t host64)
{
  return htobe64(host64);
}
inline uint32_t HostToNetwork32(uint32_t host32)
{
  return htobe32(host32);
}
inline uint16_t HostToNetwork16(uint16_t host16)
{
  return htobe16(host16);
}

inline uint16_t NetworkToHost64(uint64_t net_64)
{
  return be64toh(net_64);
}
inline uint32_t NetworkToHost32(uint32_t net_32)
{
  return be32toh(net_32);
}
inline uint16_t NetworkToHost16(uint16_t net_16)
{
  return be16toh(net_16);
}

void ToIpPort(char* buf, size_t size, const struct sockaddr* addr);
void ToIp(char* buf, size_t size, const struct sockaddr* addr);
void FromIpPort(const char* ip, uint16_t port, struct sockaddr_in* addr);
void FromIpPort(const char* ip, uint16_t port, struct sockaddr_in6* addr);



}
#endif