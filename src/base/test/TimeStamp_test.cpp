#include "../TimeStamp.hpp"

#include <fmt/format.h>

int main() {
  auto tnow = fver::base::TimeStamp::Now();
  fmt::print(
      "TimeStamp::ToFormatToday = {}\nTimeStamp::ToFormatTodayNowMs = {}\n",
      tnow.ToFormatToday(), tnow.ToFormatTodayNowMs());
  return 0;
}