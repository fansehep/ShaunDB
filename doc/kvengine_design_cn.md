## ShaunDB kv engine
LSMTree 的经典实现是 LevelDB, 我在学习完 LevelDB 的源码之后深有体会, 后面去看了一些 LSM Tree 的优化论文

### [B+ Tree 索引加速读取](https://www.usenix.org/conference/fast19/presentation/kaiyrakhmet)
- 该论文中提到了 LSM Tree 的写性能已经足够好, 但是读性能不太好, 传统的磁盘数据结构中, B+ Tree 的读性能是最好的, 所以论文中提出使用 B+ Tree 来为 Level 中的 SSTable 的 kv 做一层索引.
- ShaunDB 的实现: 使用 ```absl::btree_set<std::string_view, Comparator>``` 来做这一层映射, 当 readonly_memtable 被刷入磁盘之后, Compactor 会制作一个 MemBTreeView 放入到对应的 MemTaskWorker. 当 MemTaskWorker 处理查询请求时, 首先在 memtable 及 内存中的 bloomfilter 中寻找, 如果没有找到则在 read_only_memtable_vec(当一个 memtable 被写满之后, 会变为一个 read_only_memtable 放入到 compactor 中, 等待被 major comapct), 最后则会在 memtable_view_vec 中寻找.
- 此处做了 mmap 映射, 经典的 lsm tree 由于sstable 的kv字典有序性, key 与 key 之间都会有大量的前缀, 可以利用这种特性来做前缀压缩, 有效的减少lsm tree的空间放大, ShaunDB 目前并没有实现 sstable 的前缀压缩, 一个重要的原因是因为mmap这里做了一层视图, 后续可以考虑完成前缀压缩的功能.

### IOUring 做 compact
- iouring 是全新的统一linux 系统调用的全新异步接口, 传统的 ```AIO``` 接口实在称不上好用, 使用```epoll```做回调常常会让人不适, **iouring** 的统一异步调用接口是让人兴奋的事情.
- ShaunDB 的基于 iouring 的compact实现:
  - iouring 的封装代码位于 [shaundb_iouring](/src/util/iouring.hpp).
  - 每个 Compactor 实例都对应一个 iouring 实例, compactor 的数量是可配置的, 当 MemTaskWorker 写满一个内存表之后, 会采用 round robin 方法将 read_only_memtable 放置在不同的 compactor 之上, compactor 对于 read_only_memtable 是只读的, 所以当向iouring 提交写请求之后, 会稍后 peek 的去等待 iouring 返回, 这种异步接口需要我们在内存中维护一份map, 来做请求的对应, 当 Compactor 受到 iouring 的完成通知之后, 会去向 MemTaskWorker 提交完成结果, MemTaskWorker 此时才会去释放 read_only_memtable, 至此一份 memtable 的生命周期结束.
- 在一些测试中, iouring 的性能可能和在用户态开一个线程做异步写的性能差不太多, 对此我表示十分困惑...

### Multi Worker Multi thread Compact
- 多层 level 之间的 mino compact 常常是 lsm tree 抖动原因之一.
- 由于 ShaunDB 的设计之初并不是一个嵌入式数据库, 无需同步返回结果, 那么 multi memtable multi worker 则应运而生, 对于任何 kv 请求, db 会对 key 做hash从而散列到不同的 MemTaskWorker 之上, 每个 MemTaskWorker 都是一个线程, 他会去处理 db 散列的请求, 最后再返回给客户端结果.
- 对于请求的 hash 那么传统的 lsmtree 的level 设计则也必须也要更改, 对于每个 MemTaskWorker 来说, 他拥有被 hash 的请求的 level, 所以每一个 MemTaskWorker 每一层 level 都将只拥有一个 sstable, 对于 mino compact 也具有更好的隔离性, 从而可以使用多线程线程安全的完成 mino compact.
- 好处: 更有效的利用多线程来进行请求的处理及 compact 的完成.
- 坏处: 无法有效利用多个 memtable 来进行更好的前缀压缩, 获取更好的空间利用率.

### ShaunDB Snapshot
- 由于ShaunDB 采用了多线程且互相隔离的方式, 那么对于快照功能则需要所有的 MemTaskWorker 全部完成才可以, MemTaskWorker 负责内存快照的写入