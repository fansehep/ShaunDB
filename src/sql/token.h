#pragma once

#include "fmt/format.h"
#include "src/sql/keyword.h"
#include <fmt/core.h>
#include <optional>
#include <string>

namespace shaun {

namespace sql {

enum TokenType {
  Number,
  Str,
  Ident,
  KeyWord,
  // "."
  Period,
  // =
  Equal,
  // ">" ">="
  GreaterThan,
  GreaterThanOrEqual,
  // "<" "<="
  LessThan,
  LessThanOrEqual,
  // "<>"
  LessOrGreaterThan,
  // +
  Plus,
  // -
  Minus,
  // *
  Asterisk,
  // /
  Slash,
  // ^
  Caret,
  // %
  Percent,
  // !
  Exclamation,
  // !=
  NotEqual,
  // ?
  Question,
  // (
  LeftParen,
  // )
  RightParen,
  // ,
  Comma,
  // ;
  Semicolon,
  //
  Eof,
};

static const char *token_to_str(TokenType t) {
  switch (t) {
  case TokenType::Number:
    return "Number";
  case TokenType::Str:
    return "Str";
  case TokenType::Ident:
    return "Ident";
  case TokenType::KeyWord:
    return "KeyWord";
  case TokenType::Period:
    return "Period";
  case TokenType::Equal:
    return "Equal";
  case TokenType::GreaterThan:
    return "GreaterThan";
  case TokenType::GreaterThanOrEqual:
    return "GreaterThanOrEqual";
  case TokenType::LessThan:
    return "LessThan";
  case TokenType::LessThanOrEqual:
    return "LessThanOrEqual";
  case TokenType::LessOrGreaterThan:
    return "LessOrGreaterThan";
  case TokenType::Plus:
    return "Plus";
  case TokenType::Minus:
    return "Minus";
  case TokenType::Asterisk:
    return "Asterisk";
  case TokenType::Slash:
    return "Slash";
  case TokenType::Caret:
    return "Caret";
  case TokenType::Percent:
    return "Percent";
  case TokenType::Exclamation:
    return "Exclamation";
  case TokenType::NotEqual:
    return "NotEqual";
  case TokenType::Question:
    return "Question";
  case TokenType::LeftParen:
    return "LeftParen";
  case TokenType::RightParen:
    return "RightParen";
  case TokenType::Comma:
    return "Comma";
  case TokenType::Semicolon:
    return "Semicolon";
  case TokenType::Eof:
    return "Eof";
  default:
    return "Unknown";
  }
}

struct Token {
  std::string value;
  std::optional<int> key_word;
  TokenType type;
  Token() = default;
  Token& operator = (const Token&) = default;
  Token(const Token& t) = default;
  explicit Token(TokenType t) : type(t) {}
  explicit Token(TokenType t, int key_word_type)
      : key_word(key_word_type), type(t) {}
  explicit Token(TokenType t, int key_word_type, const char *s)
      : value(s), key_word(key_word_type), type(t) {}
  explicit Token(TokenType t, const char* s)
      : value(s), type(t) {}

      auto is_end() -> bool {
        return type == TokenType::Eof;
      }
  auto is_keyword() -> bool {
      return type == TokenType::KeyWord;
  }

  auto &set_type(TokenType t) {
    type = t;
    return *this;
  }

  auto &set_value(const std::string &v) {
    value = v;
    return *this;
  }
  auto &set_value(const std::string_view &s) {
    value = s;
    return *this;
  }

  auto &set_key_word(const int k) {
    key_word = k;
    return *this;
  }

  auto token_type() {
      return type;
  }

  auto keyword_type() {
      return key_word.value();
  }

  auto is_type(TokenType t) -> bool {
      return type == t;
  }

  auto operator==(const Token &oth) const {
    if (this->type != oth.type) {
      return false;
    }
    if (this->type == TokenType::KeyWord) {
      if (this->key_word.has_value() != oth.key_word.has_value()) {
        return false;
      }
      if (this->key_word.value() != oth.key_word.value()) {
        return false;
      }
    }
    if (this->type == TokenType::Ident) {
      if (this->value != oth.value) {
        return false;
      } 
      //
    }
    if (this->type == TokenType::Str) {
      if (this->value != oth.value) {
        return false;
      }
    }
    if (this->type == TokenType::Number) {
      if (this->value != oth.value) {
        return false;
      }
    }
    return true;
  }
};

} // namespace sql

} // namespace shaun

// for log
template <> struct fmt::formatter<shaun::sql::Token> {
  constexpr auto parse(const format_parse_context &context) {
    return context.begin();
  }
  template <typename Context>
  auto format(const shaun::sql::Token &t, Context &c) const {
    std::string res;
    if (t.key_word.has_value()) {
      res = fmt::format("keywordType: {}",
                        shaun::sql::keyword_to_str(t.key_word.value()));
    }
    if (!t.value.empty()) {
      res = fmt::format("value: {}", t.value);
    }
    return fmt::format_to(c.out(), "TokenType: {} {}",
                          shaun::sql::token_to_str(t.type), res);
  }
};
