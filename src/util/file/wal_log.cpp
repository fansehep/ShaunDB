#include "src/util/file/wal_log.hpp"

#include <fmt/format.h>

#include <cstdio>

#include "src/base/log/logging.hpp"

namespace fver {

namespace util {

namespace file {

bool WalLog::Init(const std::string &path, const std::string &filename) {
  fullFilename_ = fmt::format("{}/{}", path, filename);
  fileptr_ = std::fopen(fullFilename_.c_str(), "w+");
  if (nullptr == fileptr_) {
    LOG_WARN("wallog open {} error!", fullFilename_);
    return false;
  }
  path_ = std::string_view(fullFilename_.c_str(), path.size());
  fileName_ =
      std::string_view(fullFilename_.data() + path.size(), filename.size());
  return true;
}

void WalLog::AddRecord(const char *data, const size_t data_size) {
  assert(fileptr_ != nullptr);
  auto write_size = std::fwrite(data, sizeof(char), data_size, fileptr_);
  assert(write_size == data_size);
  std::fflush(fileptr_);
}

void WalLog::Close() {
  if (nullptr != fileptr_) {
    std::fclose(fileptr_);
  }
}

WalLog::~WalLog() {
  Close();
}

}  // namespace file

}  // namespace util

}  // namespace fver