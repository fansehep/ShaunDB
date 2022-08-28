#include "../log/Logging.hpp"

int main() {
  fver::base::log::Init("/home/fan/GitHub/UBERS/src/base/test/",
                        fver::base::log::Logger::LogLevel::kInfo,
                        fver::base::log::LogBuffer::kSmallBufferSize, 0.5);
  LOG_INFO("today i want to learn Rust");
  LOG_WARN("you can't you must learn Golang!");
  sleep(5);
  return 0;
}
