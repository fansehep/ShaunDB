#include "../log/LogFile.hpp"

#include <fmt/format.h>
#include <string.h>

#include "../TimeStamp.hpp"

using namespace fver::base;

int main() {
  log::LogFile logfile("~/GitHub/fver/base/test/");
  std::string log("hello, fver!");
  logfile.Write(log);
  const char* str = "hello, world!";
  logfile.Write(str, strlen(str));
  fmt::print("logpath = {}\n", logfile.GetCurrentLogPathName());
  fmt::print("logname = {}\n", logfile.GetCurrentLogFileName());
  return 0;
}