#ifndef SRC_NET_SERVER_H_
#define SRC_NET_SERVER_H_

#include "src/base/noncopyable.hpp"

#include <event2/event.h>

#include "src/base/log/logging.hpp"

using ::fver::base::NonCopyable;

namespace fver {
namespace net {

class NetServer : public base::NonCopyable {
 public:
  void Init() {
    // 设置 Log 为 fver_log 格式函数
    event_set_log_callback(fver_log_event_func);
  }

  static void fver_log_event_func(int severity, const char* msg) {
    switch (severity) {
      case _EVENT_LOG_DEBUG: {
        LOG_DEBUG("{}", msg);
        break;
      }
      case _EVENT_LOG_MSG: {
        LOG_INFO("{}", msg);
        break;
      }
      case _EVENT_LOG_WARN: {
        LOG_WARN("{}", msg);
        break;
      }
      case _EVENT_LOG_ERR: {
        LOG_ERROR("{}", msg);
      }
      default: {
        LOG_WARN("{}", msg);
      }
    }
  }
 private:
};

}  // namespace net

}  // namespace fver

#endif