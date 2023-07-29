#pragma once

#include <vector>
#include <mutex>
#include "src/log/conf.h"

class LogBuf {
public:

  LogBuf()
    : pre_buf_(SlogConf::kDefaultLogBufSize),
      tail_buf_(SlogConf::kDefaultLogBufSize) {
        cur_buf_ptr_ = &pre_buf_;
  }

  auto& get_mtx_ref() {
    return mtx_;
  }
  
  auto& get_curbuf_ref() {
    return &cur_buf_ptr_;
  }

  auto is_need_swap() {
    auto cur_capacity = cur_buf_ptr_->
    if (cur_buf_ptr_->size() > )
  }

private:

  std::mutex mtx_;
  std::vector<char>* cur_buf_ptr_;
  std::vector<char> pre_buf_;
  std::vector<char> tail_buf_;
};