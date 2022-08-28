#include "../log/Logging.hpp"

int main() {
  LOG_INFO("hello, fver {}", "I love you");
  LOG_WARN("I want to DeBug! {} {}", "1", pthread_self());
  LOG_EXIT("I Love Rust!");
  LOG_INFO("hello, fver {}", "I love you");
  LOG_WARN("I want to DeBug! {} {}", "1", pthread_self());
  return 0;
}
