#ifndef SRC_BASE_BACKTRACE_H_
#define SRC_BASE_BACKTRACE_H_

#include <cxxabi.h>
#include <libunwind.h>

#include <string>

namespace fver {

namespace base {

void backtrace(std::string* error_log);

static thread_local bool g_tl_entered = false;

static int GetStackTrace(void** result, int max_depth, int skip_count) {
  void* ip;
  int n = 0;
  unw_cursor_t cursor;
  unw_context_t uc;

  if (g_tl_entered) {
    return 0;
  }
  g_tl_entered = true;

  unw_getcontext(&uc);
  unw_init_local(&cursor, &uc);
  skip_count++;  // Do not include the "GetStackTrace" frame

  while (n < max_depth) {
    int ret = unw_get_reg(&cursor, UNW_REG_IP, (unw_word_t*)&ip);
    if (ret < 0) break;
    if (skip_count > 0) {
      skip_count--;
    } else {
      result[n++] = ip;
    }
    ret = unw_step(&cursor);
    if (ret <= 0) break;
  }

  g_tl_entered = false;
  return n;
}

}  // namespace base

}  // namespace fver

#endif
