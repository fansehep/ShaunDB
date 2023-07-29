#pragma once

#include <stdint.h>
// #include "src/sql/token.h"
#include <string>
#include <unordered_map>

namespace shaun {
namespace sql {

enum Keyword {
  And,
  As,
  Asc,
  Begin,
  Bool,
  Boolean,
  By,
  Char,
  Commit,
  Create,
  Cross,
  Default,
  Delete,
  Desc,
  Double,
  Drop,
  Explain,
  False,
  Float,
  From,
  Group,
  Having,
  Index,
  Infinity,
  Inner,
  Insert,
  Int,
  Integer,
  Into,
  Is,
  Join,
  Key,
  Left,
  Like,
  Limit,
  NaN,
  Not,
  Null,
  Of,
  Offset,
  On,
  Only,
  Or,
  Order,
  Outer,
  Primary,
  Read,
  References,
  Right,
  Rollback,
  Select,
  Set,
  String,
  System,
  Table,
  Text,
  Time,
  Transaction,
  True,
  Unique,
  Update,
  Values,
  Varchar,
  Where,
  Write,
  UserIdent,
};

// clang-format off
static const std::string KeywordString[] {
  "AS",
  "ASC",
  "AND",
  "BEGIN",
  "BOOL",
  "BOOLEAN",
  "BY",
  "CHAR",
  "COMMIT",
  "CREATE",
  "CROSS",
  "DEFAULT",
  "DELETE",
  "DESC",
  "DOUBLE",
  "DROP",
  "EXPLAIN",
  "FALSE",
  "FLOAT",
  "FROM",
  "GROUP",
  "HAVING",
  "INDEX",
  "INFINITY",
  "INNER",
  "INSERT",
  "INT",
  "INTEGER",
  "INTO",
  "IS",
  "JOIN",
  "KEY",
  "LEFT",
  "LIKE",
  "LIMIT",
  "NAN",
  "NOT",
  "NULL",
  "OF",
  "OFFSET",
  "ON",
  "ONLY",
  "OR",
  "ORDER",
  "OUTER",
  "PRIMARY",
  "READ",
  "REFERENCES",
  "RIGHT",
  "ROLLBACK",
  "SELECT",
  "SET",
  "STRING",
  "SYSTEM",
  "TABLE",
  "TEXT",
  "TIME",
  "TRANSACTION",
  "TRUE",
  "UNIQUE",
  "UPDATE",
  "VALUES",
  "VARCHAR",
  "WHERE",
  "WRITE",
  "USERIDENT",
};

std::string keyword_to_str(int k);
// clang-format on

class KeywordMap {
public:
  KeywordMap() {
    init();
  }

  static auto get_type(const std::string_view &str) -> Keyword;

  static auto get_type(const std::string &str) -> Keyword;

  static auto init() -> void;

  inline static std::unordered_map<std::string, Keyword> key_word_map;
};

static KeywordMap __key_word_map;

} // namespace sql

} // namespace shaun

