#ifndef SRC_DB_KEY_FORMAT_H_
#define SRC_DB_KEY_FORMAT_H_

// key_design:
//
//
//
//

#include <absl/container/btree_set.h>

#include <memory>
#include <string>
#include <string_view>

#include "src/db/comp.hpp"
#include "src/db/request.hpp"

// clang-format off

/*
 leveldb 预写日志的格式:
  | check_sum | length | type   | write_batch |
  | crc32     | 2 byte | 1 byte |   ...       |
*/




/*
  | user_key | sequence_number | value_type |
  | ? byte   |   8 byte        |  1 byte    |
*/

/*
in leveldb
  | user_key | (sequence_number) << 8 | value_type |
  | ? byte   |  8 byte                             |
个人认为: leveldb 这样做可以减少存储空间
对于性能来说, 略微有损性能, 但 sequence_number 的最大有效值就是 2 的 7 次方 -1.
*/

/*
查询一个 key 时:
查询顺序是 Memtable => Immutable Table => SStable

查询时我们需要针对于每个不同的部分定制化查询 key

look_up_key: | key_size | user_key | sequence_number | value_type |
             |  4 byte  |  ? byte  |  8 bit          |  1bit      |
memtable_key: 
             |----------------------------------------------------|
internal_key:
             |          |-----------------------------------------|
user_key:
                        |----------|
*/




/*
batch 的设计:
频繁单一的写入对于性能的影响来说还是比较大的,
引入 write_batch 可以让 cpu 更好的快速的 缓存友好的 写入到内存中去

write_batch 的设计本质上就是一个 string

write_batch_view:
  | sequence_number | count  |  value_type   | key_size  | key_val  | value_size | value  | ... | 重复
  |       8 byte    | 4 byte |  1 byte       |  4 byte   |   ? byte |    4 byte  | ? byte | ... |  
*/

/*
SSTable 设计:
 leveldb 为了能够尽量能让更多的数据获得前缀
 应该使用字典序比较器, 这样再写入的时候, 可以获得大量的相同前缀
 来减少写入的数据量

  |-->   |  Data_Block_1 |  --> | Record_1  | --> | 共享 key 的长度 | 非共享key的长度 | value 长度 | key 非共享的长度 | value 内容 |
  |         .                   | Record_2  |
  |         .                   | Record_3  |
  |         .                   | Restart[0]| --> 重启点,  leveldb 规定每隔 k 个 user_key, 
  |         .                                     将不采用前缀压缩, 而是存储整个 user_key, 用于降低查询时
  |         .                                     的复杂度, 也就是说 每隔 16 个 user_key 就存储完整的记录,
  |         .                                     同时使用 restart 数组来记录这些 restart_point 的文件偏移量
  |         .                   | Restart[1]|
  |         .
  |         .
  |--->  |    Data_Block_2 |
  |   |->|    Meta_Block_1 |  --> 用于存储 bloom_filter 的元数据, 但 leveldb 在这里做了优化,
  |   |->|    Meta_Block_3 |
  |   |  |  -------------- |
  |   -- |  Metaindex Block|  --> 记录下 Filter_Block 的起始地址和大小即可, 
  -----  |   IndexBlock    |
         |    Footer       | -----> |  Metaindex Handle |  Metaindex block 的索引 (uint64_t)
                                  |  Index Handle     |  Index block 的索引
                                  |  Padding          |
                                  |  Magic Number     |
*/

/*
Metaindex Block:

| | offset_ | size_ | compressed_type | crc32 |
| |         |       |                 |       |
用来记录 Meta_Block 的起始地址 和 偏移量
*/





/*
leveldb boom_filter storage optmize:
  leveldb 按照固定大小进行划分,  目前划分的大小是 2KB, 也就是在 Data_Block
  每 2KB 的 user_key 和 user_value 就会有一个 bloom_filter_result
  
*/

// clang-format on

namespace fver {

namespace db {

using MemBTree =
    absl::btree_set<std::string, Comparator, std::allocator<std::string>>;

// 将 set 请求 WalLog 格式化
void SetContextWalLogFormat(const std::shared_ptr<SetContext>& set_context,
                            const uint64_t number, std::string* log);

// 将 put 请求 WalLog 格式化
void PutContextWalLogFormat(const std::shared_ptr<PutContext>& set_context,
                            const uint64_t number, std::string* log);

// 将 delete 请求 WalLog 格式化
void DeleteContextWalLogFormat(
    const std::shared_ptr<DeleteContext>& set_context, const uint64_t number,
    std::string* log);

uint32_t formatDecodeFixed32(char* data);

uint64_t formatDecodeFixed64(char* data);

void formatEncodeFixed32(const uint32_t value, char* data);

void formatEncodeFixed64(const uint64_t value, char* data);

void formatEncodeFixed8(const uint8_t value, char* data);

uint8_t formatDecodeFixed8(char* data);

struct SSTableKeyValueStyle {
  std::string_view key_view;
  std::string_view value_view;
  // 是否存在
  bool isExist;
};

// 返回 memtable kvstyle => sstable kvstyle
[[nodiscard]] SSTableKeyValueStyle formatMemTableToSSTable(
    const MemBTree::iterator& iter);

[[nodiscard]] SSTableKeyValueStyle formatMemTableToSSTableStr(std::string&);

// 对 16 个 sstable_key_value_style 进行前缀压缩.
void Format16PrefixStr(const std::vector<SSTableKeyValueStyle>& sstable_vec,
                       std::string* meta_kv_str);

// 1 bit
enum ValueType {
  // 删除的标志
  kTypeDeletion = 0x0,
  kTypeValue = 0x1,
};

struct ParsedInternalKey {
  std::string_view user_key;
  uint64_t sequence_number;
  ValueType type;
};

// 为了格式化, 1 个空格
const std::string kEmpty1Space = " ";

// 为了格式化, 4 个空格
const std::string kEmpty4Space = "    ";

// 为了格式化, 8 个空格
const std::string kEmpty8Space = "        ";

// 为了格式化, 32 个空格
const std::string kEmpty32Space = "                                ";

/*
leveldb 使用 skiplist 来实现位于内存中的 Memtable
leveldb 将 user_key 和 user_value 打包成一个更大的 key

memtable_entry:
          | key_size | user_key | sequence_number | key_type | value_size |
value  | |  4 byte  |   ? byte |       8 byte    |   1 byte |   4 byte   | ?
byte |

*/

}  // namespace db

}  // namespace fver

#endif