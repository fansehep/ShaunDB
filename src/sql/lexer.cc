#include "src/sql/lexer.h"
#include "src/sql/keyword.h"
#include "src/sql/token.h"
#include <stdint.h>

namespace shaun {

namespace sql {

auto Lexer::_skip_whitespace() -> void {
  while (true) {
    if (current_read_char_ == ' ' || current_read_char_ == '\t' ||
        current_read_char_ == '\n' || current_read_char_ == '\r') {
      _read_char();
      continue;
    } else {
      break;
    }
  }
}

auto Lexer::update(const std::string &str) -> void {
  origin_sql_ = str;
  pos_ = 0;
  read_pos_ = 0;
  this->_read_char();
}

auto Lexer::update(const char *str) -> void {
  origin_sql_ = str;
  pos_ = 0;
  read_pos_ = 0;
  this->_read_char();
}

auto Lexer::next_token() -> Token {
  Token tok;
  this->_skip_whitespace();
  switch (this->current_read_char_) {
  case '=':
    tok.set_type(TokenType::Equal);
    break;
  case '.':
    tok.set_type(TokenType::Period);
    break;
  case '>':
    if (_peek_char() == '=') {
      tok.set_type(TokenType::GreaterThanOrEqual);
    } else {
      tok.set_type(TokenType::GreaterThan);
    }
    break;
  case '<':
    if (_peek_char() == '=') {
      tok.set_type(TokenType::LessThanOrEqual);
    } else if (_peek_char() == '>') {
      tok.set_type(TokenType::LessOrGreaterThan);
    } else {
      tok.set_type(TokenType::LessThan);
    }
    break;
  case '+':
    tok.set_type(TokenType::Plus);
    break;
  case '-':
    tok.set_type(TokenType::Minus);
    break;
  case '*':
    tok.set_type(TokenType::Asterisk);
    break;
  case '/':
    tok.set_type(TokenType::Slash);
    break;
  case '^':
    tok.set_type(TokenType::Caret);
    break;
  case '%':
    tok.set_type(TokenType::Percent);
    break;
  case '!':
    if (_peek_char() == '=') {
      tok.set_type(TokenType::NotEqual);
    } else {
      tok.set_type(TokenType::Exclamation);
    }
    break;
  case '?':
    tok.set_type(TokenType::Question);
    break;
  case '(':
    tok.set_type(TokenType::LeftParen);
    break;
  case ')':
    tok.set_type(TokenType::RightParen);
    break;
  case ',':
    tok.set_type(TokenType::Comma);
    break;
  case ';':
    tok.set_type(TokenType::Semicolon);
    break;
  case 0:
    tok.set_type(TokenType::Eof);
    break;
  // 解析字符串
  case '\'':
    tok.set_type(TokenType::Str).set_value(_read_string());
    break;
  case '\"':
    tok.set_type(TokenType::Str).set_value(_read_string());
    break;
  default:
    if (_is_letter(this->current_read_char_)) {
      tok.set_value(_read_identifier());
      // tok.set_type(TokenType::KeyWord);
      // tok.set_key_word(KeywordMap::get_type(tok.value));
      auto t = KeywordMap::get_type(tok.value);
      if (t == Keyword::UserIdent) {
        tok.set_type(TokenType::Ident).set_key_word(t);
      } else {
        tok.set_type(TokenType::KeyWord).set_key_word(t);
      }
      return tok;
    } else if (_is_digit(current_read_char_)) {
      tok.set_value(_read_number());
      tok.set_type(TokenType::Number);
      return tok;
    } else {
      tok.set_type(TokenType::Ident);
    }
    break;
  }
  _read_char();
  return tok;
}

auto Lexer::_peek_char() -> char {
  if (static_cast<uint64_t>(this->read_pos_) >= origin_sql_.size()) {
    return 0;
  } else {
    return origin_sql_[this->read_pos_];
  }
}

auto Lexer::_read_identifier() -> std::string_view {
  //   auto pre_pos = this->pos_;
  //   if (_is_letter(current_read_char_)) {
  //     _read_char();
  //   } else {
  //     goto READ_RETURN;
  //   }
  //   while (true) {
  //     if (_is_inner_letter(this->current_read_char_)) {
  //       _read_char();
  //     } else {
  //       break;
  //     }
  //   }
  // READ_RETURN:
  //   return std::string_view(origin_sql_.c_str() + pre_pos,
  //                           origin_sql_.c_str() + pos_);
  auto pre_pos = this->pos_;
  while (true) {
    if (_is_letter(current_read_char_) || _is_digit(current_read_char_)) {
      _read_char();
    } else {
      break;
    }
  }
  return std::string_view(origin_sql_.c_str() + pre_pos,
                          origin_sql_.c_str() + pos_);
}

auto Lexer::_read_char() -> void {
  if ((this->read_pos_) >= static_cast<int>(origin_sql_.size())) {
    current_read_char_ = 0;
  } else {
    current_read_char_ = origin_sql_[read_pos_];
  }
  pos_ = read_pos_;
  read_pos_++;
}

// 接受 [a ~ z] [A - Z] [ _ ] 的命名
// 不接受首字母为数字的自定义
auto Lexer::_is_letter(char c) -> bool {
  if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_')) {
    return true;
  }
  return false;
}

auto Lexer::_is_inner_letter(char c) -> bool {
  return _is_letter(c) || _is_digit(c);
}

auto Lexer::_is_digit(char c) -> bool { return c >= '0' && c <= '9'; }

auto Lexer::_read_number() -> std::string_view {
  auto pre_pos = pos_;
  while (true) {
    if (_is_digit(current_read_char_)) {
      _read_char();
    } else if (_is_decimal_point(current_read_char_)) {
      _read_char();
    } else {
      break;
    }
  }
  return std::string_view(origin_sql_.c_str() + pre_pos,
                          origin_sql_.c_str() + pos_);
}

auto Lexer::_read_string() -> std::string_view {
  auto pre_pos = pos_ + 1;
  while (true) {
    _read_char();
    if (current_read_char_ == '\'' || current_read_char_ == '\"' ||
        current_read_char_ == 0) {
      break;
    }
  }
  return std::string_view(origin_sql_.c_str() + pre_pos,
                          origin_sql_.c_str() + pos_);
}

auto Lexer::_is_decimal_point(char c) -> bool {
  return c == '.';
}


} // namespace sql

} // namespace shaun