## ShaunDB 网络模块设计

1. ShaunDB 的网络模块在 [ShaunDB/src/net/](../src/net/net_server.hpp) 下.
2. ShaunDB 的网络模块基于libevent. (PS: 为什么使用 libevent, answer: 我想偷懒).
3. 所以大概上设计没有什么太多创新可言, 都是 epoll 的那种回调老套路.
4. 封装了重复定时器, 一次性定时器, TCPServer, TCPClient, Connector(主动连接者) 等等 class. 使用 map 来进行所有的 client 的连接管理.