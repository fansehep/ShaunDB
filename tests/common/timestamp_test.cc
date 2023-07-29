

#include "fmt/format.h"
#include "src/common/timestamp.h"

int main() {
  shaun::TimeStamp tt;
  tt.update();
  fmt::print("to_day:     {}\n", tt.to_day());
  fmt::print("to_day_ms:  {}\n", tt.to_day_ms());
  fmt::print("to_day_us:  {}\n", tt.to_day_us());
  fmt::print("to_day_min: {}\n", tt.to_day_min());
}