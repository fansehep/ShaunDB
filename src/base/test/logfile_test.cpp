#include "src/base/log/logfile.hpp"

int main () {
  fver::base::log::LogFile logfile("/home/fan/GitHub/fver/src/base/test");
  logfile.Write("123213123");
  logfile.Write("131231");
  logfile.Sync();
  return 0;
}