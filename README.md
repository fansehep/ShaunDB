<div align=left> <img src="doc/image/shaundb_icon.jpg" width = 45%>

# ShaunDB

ShaunDB 是一款简单支持 Redis协议的KV数据库, 该项目旨在满足我对于一些 lsm tree 论文的复现及优化. 及结合我此前学过的一些知识, 简单构建一个kv系统.

```Shaun``` 该名字来源于[```<<Shaun the Sheep>>```](https://zh.wikipedia.org/zh-hant/%E5%B0%8F%E7%BE%8A%E8%82%96%E6%81%A9) (小羊肖恩) 该动画中主角名字.

## 架构:
![](/doc/image/shaundb_design.png)

设计:
- 日志设计:
  
  ShaunDB 内部拥有日志系统, 详情请看 [shaundb_log_design](/doc/log_design_cn.md).

- 网络部分:

  基于 libevent 封装了 TCPServer, TCPClient, 以及一次性, 重复定时器.

- kv 引擎:

  lsm tree 这种架构固然可以带来较好的写性能, 但为了减少空间放大的Compaction 常常被人诟病, 以及较弱的读性能也是优化的重点之一.
  