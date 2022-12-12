<div align=left> <img src="doc/image/shaundb_icon.jpg" width = 25%>

# **ShaunDB**

ShaunDB 是一款简单支持 Redis协议的KV数据库, 该项目旨在满足我对于一些 lsm tree 论文的复现及优化. 及结合我此前学过的一些知识, 简单构建一个kv系统.

```Shaun``` 该名字来源于[```<<Shaun the Sheep>>```](https://zh.wikipedia.org/zh-hant/%E5%B0%8F%E7%BE%8A%E8%82%96%E6%81%A9) (小羊肖恩) 该动画中主角名字.

## **架构**
![](/doc/image/shaundb_design.png)

## **设计**
- 日志设计:
  
  ShaunDB 内部拥有日志系统, 详情请见 [shaundb_log_design](/doc/log_design_cn.md).

- 网络部分:

  基于 libevent 封装了 TCPServer, TCPClient, 以及一次性, 重复定时器.

- kv 引擎:

  lsm tree 这种架构固然可以带来较好的写性能, 但为了减少空间放大的Compaction 常常被人诟病, 以及较弱的读性能也是优化的重点之一.
  详情请见 [shaundb_kvengine_design](/doc/kvengine_design_cn.md).

- cache 设计:

  ShaunDB 有独特的缓存设计, 详情请见 [shaundb_klrucache_design](/doc/klrucache_design_cn.md).

## **构建**
  ShaunDB 目前只支持 Bazel 构建, 需要兼容5.3.2的Bazel版本.
  - 默认使用 clang 进行编译, 需要支持 C++20 以上的编译器.
  - 依赖, 一些测试依赖于 tcmalloc, 及静态依赖了 libevent, 后续有空将其加入到Bazel 构建中.
  - 目前由于使用 iouring 做 compact, 只支持 ```linux kernel version >= 5.1```  的发行版.
  ```bash
  git submodule init
  bazel build "..."
  ```

## **Countribute**
  ShaunDB 目前仍然处于开发阶段.
  - ShaunDB 完全遵循 [Google C++ code style](https://google.github.io/styleguide/cppguide.html) 且使用 C++20 开发.

## **LICENSE**
  ShaunDB 以[GNU GENERAL PUBLIC LICENSE 3.0](LICENSE) 进行许可.