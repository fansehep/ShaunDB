
#include "src/log/slog.h"

const char* ddd[] = {
  "123",
  "456",
  "daikhdukasd",
};

int main() {
  Info("hello, world");
  Warn("today is good day", rand());
  Debug("djailsdjlad {} {} ", rand(), "123");
  Trace("sdjilasdilafilajsilf {} {}", rand(), rand());
  Error("djsialkdaskhnfkahnfiajsdilasd {} {}", rand(), rand());
  Dinfo("12j3ijdaisd");
  for (auto it : ddd) {
    Info("distance: {}", it - ddd[0]);
  }
  
  return 0;
}