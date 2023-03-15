### ShaunDB::Raft 设计

Paxos 的论文实在是难以理解, 无法在工程上友好实现, 固然 Chuppy 的作者说, 世界上只有一种共识算法就是 Paxos, 但 Raft 的工程性和易于理解的程度都高于 Paxos. 以及 Raft 有着 Etcd 这样的优秀开源工业级实现, 所以选择 Raft 也不错.

笔者学习过 MIT6.824, lab2 是使用 go 语言完成一个简单的Raft 算法. 为加深理解, 我决定在 ShaunDB 中 Port 出来一份.

ShaunDB 使用 boost::fiber 协程库来提高并发.

boost::fiber 已经是一个比较高级的抽象,  我只对其做简单的封装, 位于 [shuandb_fiber_define](/src/raft/fiber_define.hpp).

整体架构:

![](/doc/image/shaundb_arch.png)


- 目前已经完成 Raft::RequestVote 逻辑.

- ShaunDB::RaftLog 的实现将参考 Tikv::RaftLog 的设计.

