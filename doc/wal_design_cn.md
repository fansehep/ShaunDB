## ShaunDB Wal design
预写日志几乎是现代所有的引擎必备的一部分.
ShaunDB 也不例外, ShaunDB 的预写日志的设计为环形日志设计.
当写到达阈值时(默认是 72MB) 将会 ```seek``` 到文件开始继续写入.
实现相对比较简单, 但是 WAL 恢复时比较麻烦, 需要找到环形WAL 的头尾.

![](./image/shaundb_wal_design.png)
