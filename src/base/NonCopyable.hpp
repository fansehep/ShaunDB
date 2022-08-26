#ifndef SRC_BASE_NONCOPYABLE_H_
#define SRC_BASE_NONCOPYABLE_H_
#include <boost/core/noncopyable.hpp>
namespace fver {
namespace base {

class NonCopyable {
 protected:
  constexpr NonCopyable() = default;
  ~NonCopyable() = default;

 public:
  NonCopyable(const NonCopyable&) = delete;
  NonCopyable& operator=(const NonCopyable&) = delete;
};
}  // namespace base
}  // namespace fver

#endif