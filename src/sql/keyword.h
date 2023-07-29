#pragma once

#include <unordered_map>

enum keyword {
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
};

const char* KeywordString[] {
  "AS",
  "ASC",
  "AND",
  "BEGIN",
  "BOOL"
}



class KeywordMap {
public:
  KeywordMap();

  auto is_keyword(const std::string& str) -> bool;




  static std::unordered_map<std::string, keyword> key_word_map_;
};

KeywordMap ___GlobalMap;