#ifndef SRC_DB_WAL_WRITER_H_
#define SRC_DB_WAL_WRITER_H_

#include <memory>
#include <thread>

#include "src/base/noncopyable.hpp"
#include "src/db/request.hpp"
#include "src/db/status.hpp"
#include "src/util/file/wal_log.hpp"

using ::fver::base::NonCopyable;

// 分为多个 block,
/*
| block_1 | block_2 | block_3 | block_4 |


一个 block 有多条记录,

Record :
check_sum (crc32) | length (2 byte) | type (1 byte) | write_batch_.rep_ |

*/

namespace fver {

namespace db {

/*
 * 使用环形文件日志. 当超过阈值回溯到文件最开始的地方开始继续写入即可
 *
 *
 */

// 每条预写日志通过写入 "\r\n\r\n" 来进行一次标记.
const std::string kFlagLogEnd = "\r\n\r\n\x00\x00\x00\x00";
// 回复使利用 std::string::find
// 来进行快速回复

// 将 set 请求 WalLog 格式化
void SetContextWalLogFormat(
    const std::shared_ptr<SetContext>& set_context, const uint64_t number,
    std::string* log);

// 将 put 请求 WalLog 格式化
void PutContextWalLogFormat(
    const std::shared_ptr<PutContext>& set_context, const uint64_t number,
    std::string* log);

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

// 默认环形预写日志的容量, 默认为 64MB 当超出 64MB 时 seek 到开头继续写入
static constexpr uint32_t kDefaultWalLogSize = 64 * 1024 * 1024;

// 为了格式化, 4 个空格
const std::string kEmpty4Space = "    ";

// 为了格式化, 8 个空格
const std::string kEmpty8Space = "        ";

class WalWriter : public NonCopyable {
 public:
  WalWriter();
  ~WalWriter();
  // 预写日志 初始化
  /*
   * @path: 日志存放路径
   * @file_name: 日志名称
   */
  bool Init(const std::string& path, const std::string& file_name,
            Status* status);

  // 追加日志
  bool AddRecord(const std::string_view& record);

  // 设置预写日志容量
  void SetWalLogDefultSize(const uint32_t log_size);

  // for test
  int ReadLog(char* data, const size_t size);

 private:
  // 恢复日志重启
  /*
    当 lsm tree 由于断电各种故障时, 我们需要使用 wal_log 日志进行恢复
    单条日志格式中已经有 sequence_number, 根据序号判断即可.
    但由于是环形缓冲区, 我们应该在这里的恢复做出一些判断
    1. 如果日志没有写满, 即打开的当前的wal_log_size < 64MB, 顺序读一遍即可
    2. 如果日志已经写满了, 则我们需要找到 环形缓冲区的 头 / 尾 再进行恢复
       . 找到 序列号相差很大的那两个记录.
  */

  // 当前已经写入的长度
  uint32_t current_log_size_;
  // wal log 的默认大小
  uint32_t wal_log_size_;
  // 预写日志所对应的文件
  util::file::WalLog wal_file_;
};

}  // namespace db

}  // namespace fver

#endif