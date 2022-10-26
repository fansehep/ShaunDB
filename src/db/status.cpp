#include "src/db/status.hpp"

namespace fver {

namespace db {

Status::Status()
    : code_(StatusCode::kOk), code_ptr_(statuslev[StatusCode::kOk]) {}

void Status::setCode(StatusCode code) { code_ = code; }

StatusCode Status::getCode() { return code_; }

std::string_view Status::getCodeStr() {
  code_ptr_ = statuslev[static_cast<int>(code_)];
  return code_ptr_;
}

}  // namespace db

}  // namespace fver