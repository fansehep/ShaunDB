#include "fmt/format.h"

#include <string>
#include <string_view>
#include <vector>

int main() {
  auto fmt_str = fmt::format("{:>5} {} {}", 1, 231, 123);
  fmt::print("{}\n", fmt_str);
  fmt_str = fmt::format("{:>5} {} {}", 123, 13, 123);
  fmt::print("{}\n", fmt_str);
  std::vector<char> buf;
  buf.reserve(1024);
  buf.push_back('1');
  fmt::print("{} {}\n", buf.size(), buf.capacity());
  return 0;
}