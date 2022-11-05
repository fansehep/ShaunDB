#ifndef SRC_DB_COMPACTOR_H_
#define SRC_DB_COMPACTOR_H_

#include "src/base/log/logging.hpp"
#include "src/base/noncopyable.hpp"

namespace fver {

namespace db {

/*
 当 Memtable 写到一定容量之后, 会变成 readonly
 Memtable 会使用一个新的表, 来进行写入, 但是
 Compactor 将会负责 将 readonly Memtable 写入到文件中
 单个 Memtable => SStable,
 Compaction 操作分为
  1. Minor_Compaction: 主动将 Memtable => SStable
  2. 当 SStable 到达一定层级时, 多个不同的 SStable 中间会有
     key 重叠, 为了减少这部分的写入放大. 需要将 多个 SStable
     合并成一个 SStable
*/

// clang-format off

/*
       Memtable_1         Memtable_2          Memtable_3
      |--------|          |--------|          |--------|
      |--------|          |--------|          |--------|
      |--------|          |--------|          |--------|
      |--------|          |--------|          |--------|
          ||                  ||                  ||
          ||                  ||                  ||
          ||                  ||                  ||
      |--------|          |--------|          |--------|
      |--------|          |--------|          |--------|
      |--------|          |--------|          |--------|
      |--------|          |--------|          |--------|
       read_only           read_only           read_only
          |                   |                   |
          |                   |                   |
          \                   |                   /
           \                  |                  /
      |-------------------------------------------------|
      |                     Compactor                   |
      |-------------------------------------------------|
      |                 multi_thread_compact            |
      |-------------------------------------------------|
          ||                 ||                 ||
          ||                 ||                 ||
          ||                 ||                 ||
      |-------|         |---------|         |---------| -- Memtable_3_SSTable_0
      |-------|         |---------|         |---------| -- Memtable_3_SSTable_1
      |-------|         |---------|         |---------| -- Memtable_3_SSTable_2
*/

// clang-format on

// 默认先使用 2 个线程进行 4 个 Memtable 的 Compaction
// TODO: 应该提供一个配置文件来进行设置
constexpr static uint32_t kDefaultCompactor_N = 2;


class Compactor : public NonCopyable {
 public:
 private:
  // TODO: 一个线程真的够用吗
};

}  // namespace db

}  // namespace fver

#endif