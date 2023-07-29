#pragma once


namespace shaun {

enum TokenType {
  Number,
  String,
  Ident,
  Keyword,
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
};

struct TokenNumber {
  std::string n;  
};

struct TokenString {
  std::string s;
};

struct TokenIdent {
  std::string ident;
};

struct TokenKeyword {
  std::string keyword;
};




}