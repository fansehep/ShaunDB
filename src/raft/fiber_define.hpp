#ifndef SRC_RAFT_FIBER_DEFINE_H_
#define SRC_RAFT_FIBER_DEFINE_H_

#include <thread>
#include "boost/fiber/all.hpp"
#include "fmt/format.h"

/**
 * @_@ i am also a goer?
 */
using go = boost::fibers::fiber;

// for shaundb::log print
template <>
struct fmt::formatter<boost::fibers::fiber::id> {
  constexpr auto parse(const format_parse_context& context) {
    return context.begin();
  }
  template <typename Context>
  constexpr auto format([[maybe_unused]] const boost::fibers::fiber::id& id,
                        Context& cont) const {
    return fmt::format_to(cont.out(), "{}",
                          (void*)boost::fibers::context::active());
  }
};

template <>
struct fmt::formatter<std::thread::id> {
  constexpr auto parse(const format_parse_context& context) {
    return context.begin();
  }
  template <typename Context>
  constexpr auto format(const std::thread::id& id, Context& cont) const {
    return fmt::format_to(cont.out(), "{}", *((uint64_t*)&id));
  }
};


namespace fiber {
  using fmutex = boost::fibers::mutex;
  using ftimed_mutex = boost::fibers::timed_mutex;
  using fcond = boost::fibers::condition_variable;
  using fcond_any = boost::fibers::condition_variable_any;
}

#define fiber_yield boost::fiber::yield();


/**
 * @brief thread_n: boost::fiber M:N 所占用 N 线程的个数
 * @brief
 */
static void init_fiber(const uint32_t thread_n) {}

#endif