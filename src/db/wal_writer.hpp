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
const std::string kFlagLogEnd = "\r\n\r\n";
// 回复使利用 std::string::find
// 来进行快速回复

// 将 set 请求 WalLog 格式化
extern void SetContextWalLogFormat(
    const std::shared_ptr<SetContext>& set_context, std::string* log);

// 将 put 请求 WalLog 格式化
extern void PutContextWalLogFormat(
    const std::shared_ptr<SetContext>& set_context, std::string* log);

// 将 delete 请求 WalLog 格式化
extern void DeleteContextWalLogFormat(
    const std::shared_ptr<SetContext>& set_context, std::string* log);

static constexpr uint32_t kDefaultWalLogSize = 32 * 1024 * 1024;

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