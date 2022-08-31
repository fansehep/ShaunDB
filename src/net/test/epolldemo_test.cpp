#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>


//* 从一个简单的同步epoll服务器抽象出一个完整的模型

int creat_socket(char *ip, char *port) {
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    perror("socket");
    exit(2);
  }

  //调用setsockopt使当server先断开时避免进入TIME_WAIT状态,\
     将其属性设定为SO_REUSEADDR，使其地址信息可被重用
  int opt = 1;
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    perror("setsockopt");
    exit(3);
  }

  struct sockaddr_in local;

  local.sin_family = AF_INET;
  local.sin_port = htons(atoi(port));
  local.sin_addr.s_addr = inet_addr(ip);

  if (bind(sock, (struct sockaddr *)&local, sizeof(local)) < 0) {
    perror("bind");
    exit(4);
  }

  if (listen(sock, 5) < 0) {
    perror("listen");
    exit(5);
  }

  printf("listen and bind succeed\n");

  return sock;
}

int set_noblock(int sock) {
  int fl = fcntl(sock, F_GETFL);
  return fcntl(sock, F_SETFL, fl | O_NONBLOCK);
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("please use:%s [ip] [port]", argv[0]);
    exit(1);
  }
  int listen_sock = creat_socket(argv[1], argv[2]);

  int epoll_fd = epoll_create(256);
  if (epoll_fd < 0) {
    perror("epoll creat");
    exit(6);
  }

  struct epoll_event ep_ev;
  ep_ev.events = EPOLLIN;  //数据的读取
  ep_ev.data.fd = listen_sock;

  //添加关心的事件
  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_sock, &ep_ev) < 0) {
    perror("epoll_ctl");
    exit(7);
  }

  struct epoll_event ready_ev[128];  //申请空间来放就绪的事件。
  int maxnum = 128;
  int timeout = 1000;  //设置超时时间，若为-1，则永久阻塞等待。
  int ret = 0;

  int done = 0;
  while (!done) {
    switch (ret = epoll_wait(epoll_fd, ready_ev, maxnum, timeout)) {
      case -1:
        perror("epoll_wait");
        break;
      case 0:
        printf("time out...\n");
        break;
      default:  //至少有一个事件就绪
      {
        int i = 0;
        for (; i < ret; ++i) {
          //判断是否为监听套接字，是的话accept
          int fd = ready_ev[i].data.fd;
          if ((fd == listen_sock) && (ready_ev[i].events & EPOLLIN)) {
            struct sockaddr_in remote;
            socklen_t len = sizeof(remote);

            int accept_sock =
                accept(listen_sock, (struct sockaddr *)&remote, &len);
            if (accept_sock < 0) {
              perror("accept");
              continue;
            }
            printf("accept a client..[ip]: %s,[port]: %d\n",
                   inet_ntoa(remote.sin_addr), ntohs(remote.sin_port));
            //将新的事件添加到epoll集合中
            ep_ev.events = EPOLLIN | EPOLLET;
            ep_ev.data.fd = accept_sock;

            set_noblock(accept_sock);

            if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, accept_sock, &ep_ev) < 0) {
              perror("epoll_ctl");
              close(accept_sock);
            }
          } else {  //普通IO
            if (ready_ev[i].events & EPOLLIN) {
              //申请空间同时存文件描述符和缓冲区地址

              char buf[102400];
              memset(buf, '\0', sizeof(buf));

              ssize_t _s = recv(fd, buf, sizeof(buf) - 1, 0);
              if (_s < 0) {
                perror("recv");
                continue;
              } else if (_s == 0) {
                printf("remote close..\n");
                //远端关闭了，进行善后
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
                close(fd);
              } else {
                //读取成功，输出数据
                printf("client# %s", buf);
                fflush(stdout);

                //将事件改写为关心事件，进行回写
                ep_ev.data.fd = fd;
                ep_ev.events = EPOLLOUT | EPOLLET;

                //在epoll实例中更改同一个事件
                epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ep_ev);
              }
            } else if (ready_ev[i].events & EPOLLOUT) {
              const char *msg =
                  "HTTP/1.1 200 OK \r\n\r\n<h1> hi girl </h1>\r\n";
              send(fd, msg, strlen(msg), 0);
              epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
              close(fd);
            }
          }
        }
      } break;
    }
  }
  close(listen_sock);
  return 0;
}
