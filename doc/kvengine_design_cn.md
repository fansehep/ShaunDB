## ShaunDB kv engine
LSMTree 的经典实现是 LevelDB, 我在学习完 LevelDB 的源码之后深有体会, 后面去看了一些 LSM Tree 的优化论文

### [B+ Tree 索引加速读取](https://www.usenix.org/conference/fast19/presentation/kaiyrakhmet).
- 该论文中提到了 LSM Tree 的写性能已经足够好, 但是读性能不太好, 传统的磁盘数据结构中, B+ Tree 的读性能是最好的, 所以论文中提出使用 B+ Tree 来为 Level 中的 SSTable 的 kv 做一层索引.
- ShaunDB 的实现: 使用 ```absl::btree_set<std::string_view, Comparator>``` 来做这一层映射, 当 readonly_memtable 被刷入磁盘之后, Compactor 会制作一个 MemBTreeView 放入到对应的 MemTaskWorker.