<div align=left> <img src="doc/image/shaundb_icon.jpg" width = 25%>

# **ShaunDB**

ShaunDB 是一款简单支持 Redis协议的KV数据库, 该项目旨在满足我对于一些 lsm tree 论文的复现及优化. 及结合我此前学过的一些知识, 简单构建一个kv系统.

```Shaun``` 该名字来源于[```<<Shaun the Sheep>>```](https://zh.wikipedia.org/zh-hant/%E5%B0%8F%E7%BE%8A%E8%82%96%E6%81%A9) (小羊肖恩) 该动画中主角名字.

## **单机架构**
![](/doc/image/shaundb_design.jpg)


## **Raft多副本架构**

![](/doc/image/shaundb_arch.png)


## **设计**
- 日志设计:
  
  ShaunDB 内部拥有日志系统, 详情请见 [shaundb_log_design](/doc/log_design_cn.md).

- 网络部分:

  ShaunDB 由于需要兼容 Redis 协议, 所以做了简单的 网络部分实现, 详情请见 [shaundb_net_design](/doc/net_design_cn.md).

- kv 引擎:

  lsm tree 这种架构固然可以带来较好的写性能, 但为了减少空间放大的Compaction 常常被人诟病, 以及较弱的读性能也是优化的重点之一.
  详情请见 [shaundb_kvengine_design](/doc/kvengine_design_cn.md).

- cache 设计:

  ShaunDB 有独特的缓存设计, 详情请见 [shaundb_klrucache_design](/doc/klrucache_design_cn.md).

- Raft 设计

  ShaunDB 节点之间通信使用 gRPC, 使用 boost::fiber 协程库来提高并发, 实现位于 [shaundb_raft_module](/src/raft/), 详见 [shaundb_raft_design](/doc/raft_design_cn.md).

## **构建**
  ShaunDB 目前只支持 Bazel 构建, 需要兼容5.3.2的Bazel版本.
  - 默认使用 clang 进行编译, 需要支持 C++20 以上的编译器.
  - 依赖, 一些测试依赖于 tcmalloc, 及静态依赖了 libevent, 后续有空将其加入到Bazel 构建中.
  - 目前由于使用 iouring 做 compact, 只支持 ```linux kernel version >= 5.1```  的发行版.
  ```bash
  git submodule init
  git submodule update
  bazel build "..."
  ```

## **Countribute**
  ShaunDB 目前仍然处于开发阶段.
  - ShaunDB 完全遵循 [Google C++ code style](https://google.github.io/styleguide/cppguide.html) , 使用 2 空格进行缩进, 使用 C++20 开发.

## **LICENSE**
  ShaunDB 以[GNU GENERAL PUBLIC LICENSE 3.0](LICENSE) 进行许可.