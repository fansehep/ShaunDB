#include "src/db/status.hpp"


namespace fver {


namespace db {


void Status::setCode(StatusCode code) {
  code_ = code;
}

StatusCode Status::getCode() {
  return code_;
}

std::string_view Status::getCodeStr() {
  code_ptr_ = statuslev[static_cast<int>(code_)];
  return code_ptr_;
}



}



}