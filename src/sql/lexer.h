#pragma once

#include "src/sql/token.h"
#include <string>

namespace shaun {

namespace sql {

class Lexer {
public:
  auto next_token() -> Token;

  explicit Lexer(const char *origin_source) : origin_sql_(origin_source) {
    pos_ = 0;
    read_pos_ = 0;
    this->_read_char();
  }

  explicit Lexer(const std::string &origin_source)
      : origin_sql_(origin_source) {
    pos_ = 0;
    read_pos_ = 0;
    this->_read_char();
  }

  auto update(const std::string& str) -> void;

  auto update(const char* str) -> void;

private:
  auto _skip_whitespace() -> void;

  auto _peek_char() -> char;

  auto _is_letter(char c) -> bool;

  auto _is_digit(char c) -> bool;

  auto _read_identifier() -> std::string_view;

  auto _read_number() -> std::string_view;

  auto _read_char() -> void;

  auto _read_string() -> std::string_view;

  auto _is_inner_letter(char) -> bool;

  auto _is_decimal_point(char) -> bool;

  std::string origin_sql_;
  int pos_;
  int read_pos_;
  char current_read_char_;
};

} // namespace sql

} // namespace shaun