#include "src/base/backtrace.hpp"
#include <asm-generic/errno-base.h>
#include <fmt/format.h>
extern "C" {
#include <assert.h>
}

void fver::base::backtrace(std::string* error_log) {
  assert(nullptr != error_log);
  unw_cursor_t cursor;
  unw_context_t context;

  unw_getcontext(&context);
  unw_init_local(&cursor, &context);
  while (unw_step(&cursor) > 0) {
    unw_word_t offset, pc;
    unw_get_reg(&cursor, UNW_REG_IP, &pc);
    if (pc == 0) {
      break;
    }
    *error_log = fmt::format("0x{}", pc);
    char sym[256];
    if (unw_get_proc_name(&cursor, sym, sizeof(sym), &offset) == 0) {
      char* nameptr = sym;
      int status;
      char* demangled = abi::__cxa_demangle(sym, nullptr, nullptr, &status);
      if (status == 0) {
        nameptr = demangled;
      }
      auto logstr = fmt::format("0x{}: {} 0x{}\n", pc, nameptr, offset);
      error_log->append(logstr);
      std::free(demangled);
    } else {
      std::printf(" -- error: unable to obtain symbol name for this frame\n");
    }
  }
}
