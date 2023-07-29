#include "src/log/slog.h"
#include "src/sql/keyword.h"
#include "src/sql/lexer.h"
#include "src/sql/token.h"
#include "gtest/gtest.h"

using shaun::sql::Keyword;
using shaun::sql::Token;
using shaun::sql::TokenType;

namespace shaun::sql {

class LexerTest : public ::testing::Test {
public:
  LexerTest() = default;
  ~LexerTest() = default;
  void SetUp() override {
    // shaun::sql::KeywordMap::init();
  }
  void TearDown() override {}
};

TEST_F(LexerTest, BasicTest1) {
  const std::string sql1 = "select * from TABLE_NAME_1;";
  shaun::sql::Lexer lexer(sql1);
  std::vector<Token> res = {
      Token(TokenType::KeyWord, Keyword::Select),
      Token(TokenType::Asterisk),
      Token(TokenType::KeyWord, Keyword::From),
      Token(TokenType::Ident, "TABLE_NAME_1"),
      Token(TokenType::Semicolon),
      Token(TokenType::Eof),
  };
  int i = 0;
  for (auto &it : res) {
    auto t = lexer.next_token();
    if (t != it) {
      Warn("idx: {} expected: {} but get: {}", i, it, t);
    }
    ASSERT_EQ(t, it);
    i++;
  }
}

TEST_F(LexerTest, BasicTest2) {
  const char *sql = "CREATE TABLE movie ("
                    "id INTEGER PRIMARY KEY,"
                    "title STRING NOT NULL,"
                    "release_year INTEGER INDEX,"
                    "imdb_id STRING INDEX UNIQUE,"
                    "bluray BOOLEAN NOT NULL DEFAULT TRUE"
                    ");";
  Lexer lexer(sql);
  std::vector<Token> res = {
      Token(TokenType::KeyWord, Keyword::Create),
      Token(TokenType::KeyWord, Keyword::Table),
      Token(TokenType::Ident, "movie"),
      Token(TokenType::LeftParen),
      Token(TokenType::Ident, "id"),
      Token(TokenType::KeyWord, Keyword::Integer),
      Token(TokenType::KeyWord, Keyword::Primary),
      Token(TokenType::KeyWord, Keyword::Key),
      Token(TokenType::Comma),
      Token(TokenType::Ident, "title"),
      Token(TokenType::KeyWord, Keyword::String),
      Token(TokenType::KeyWord, Keyword::Not),
      Token(TokenType::KeyWord, Keyword::Null),
      Token(TokenType::Comma),
      Token(TokenType::Ident, "release_year"),
      Token(TokenType::KeyWord, Keyword::Integer),
      Token(TokenType::KeyWord, Keyword::Index),
      Token(TokenType::Comma),
      Token(TokenType::Ident, "imdb_id"),
      Token(TokenType::KeyWord, Keyword::String),
      Token(TokenType::KeyWord, Keyword::Index),
      Token(TokenType::KeyWord, Keyword::Unique),
      Token(TokenType::Comma),
      Token(TokenType::Ident, "bluray"),
      Token(TokenType::KeyWord, Keyword::Boolean),
      Token(TokenType::KeyWord, Keyword::Not),
      Token(TokenType::KeyWord, Keyword::Null),
      Token(TokenType::KeyWord, Keyword::Default),
      Token(TokenType::KeyWord, Keyword::True),
      Token(TokenType::RightParen),
      Token(TokenType::Semicolon),
      Token(TokenType::Eof),
  };
  int i = 0;
  for (auto &it : res) {
    auto t = lexer.next_token();
    if (t != it) {
      Warn("{} expected: {} but get: {}", i, it, t);
    }
    ASSERT_EQ(t, it);
    i++;
  }
  const char *sql2 = "INSERT INTO movie "
                     "(id, title, release_year) "
                     "VALUES "
                     "(1, 'Sicario', 2015), "
                     "(2, 'Stalker', 1979), "
                     "(3, 'Her', 2013);";
  lexer.update(sql2);
  res = {
      Token(TokenType::KeyWord, Keyword::Insert),
      Token(TokenType::KeyWord, Keyword::Into),
      Token(TokenType::Ident, "movie"),
      Token(TokenType::LeftParen, "("),
      Token(TokenType::Ident, "id"),
      Token(TokenType::Comma),
      Token(TokenType::Ident, "title"),
      Token(TokenType::Comma),
      Token(TokenType::Ident, "release_year"),
      Token(TokenType::RightParen, ")"),
      Token(TokenType::KeyWord, Keyword::Values),
      Token(TokenType::LeftParen),
      Token(TokenType::Number, "1"),
      Token(TokenType::Comma),
      Token(TokenType::Str, "Sicario"),
      Token(TokenType::Comma),
      Token(TokenType::Number, "2015"),
      Token(TokenType::RightParen),
      Token(TokenType::Comma),
      Token(TokenType::LeftParen),
      Token(TokenType::Number, "2"),
      Token(TokenType::Comma),
      Token(TokenType::Str, "Stalker"),
      Token(TokenType::Comma),
      Token(TokenType::Number, "1979"),
      Token(TokenType::RightParen),
      Token(TokenType::Comma),
      Token(TokenType::LeftParen),
      Token(TokenType::Number, "3"),
      Token(TokenType::Comma),
      Token(TokenType::Str, "Her"),
      Token(TokenType::Comma),
      Token(TokenType::Number, "2013"),
      Token(TokenType::RightParen),
      Token(TokenType::Semicolon),
      Token(TokenType::Eof),
  };
  i = 0;
  for (auto &it : res) {
    auto t = lexer.next_token();
    if (t != it) {
      Warn("{} expected: {} but get: {}", i, it, t);
    }
    ASSERT_EQ(t, it);
    i++;
  }

  //
  const char *sql3 =
      "INSERT INTO movies VALUES "
      "(1,  'Stalker',             1, 1, 1979, 8.2), "
      "(2,  'Sicario',             2, 2, 2015, 7.6), "
      "(10, 'Inception',           4, 1, 2010, 8.8), "
      "(11, 'Lost in Translation', 5, 4, 2003, 7.7), "
      "(12, 'Eternal Sunshine of the Spotless Mind', 5, 3, 2004, 8.3);";
  res = {
    Token(TokenType::KeyWord, Keyword::Insert),
    Token(TokenType::KeyWord, Keyword::Into),
    Token(TokenType::Ident, "movies"),
    Token(TokenType::KeyWord, "VALUES"),
    Token(TokenType::LeftParen),
    Token(TokenType::Number, "1"),
    Token(TokenType::Comma),
    Token(TokenType::Str, "Stalker"),
    Token(TokenType::Comma),
    Token(TokenType::Number, "1"),
    Token(TokenType::Comma),
    Token(TokenType::Number, "1"),
    Token(TokenType::Comma),
    Token(TokenType::Number, "1979"),
    Token(TokenType::Comma),
    Token(TokenType::Number, "8.2"),
    Token(TokenType::RightParen),
    Token(TokenType::Comma),
    Token(TokenType::LeftParen),
    Token(TokenType::Number, "2"),
    Token(TokenType::Comma),
    Token(TokenType::Str, "Sicario"),
    Token(TokenType::Comma),
    Token(TokenType::Number, "2"),
    Token(TokenType::Comma),
    Token(TokenType::Number, "2"),
    Token(TokenType::Comma),
    Token(TokenType::Number, "2015"),
    Token(TokenType::Comma),
    Token(TokenType::Number, "7.6"),
    Token(TokenType::RightParen),
    Token(TokenType::Comma),
    Token(TokenType::LeftParen),
    Token(TokenType::Number, "1"),
    Token(TokenType::Comma),
    Token(TokenType::Str, "Stalker"),
    Token(TokenType::Comma),
    Token(TokenType::Number, "1"),
    Token(TokenType::Comma),
    Token(TokenType::Number, "1"),
    Token(TokenType::Comma),
    Token(TokenType::Number, "1979"),
    Token(TokenType::Comma),
    Token(TokenType::Number, "8.2"),
    Token(TokenType::RightParen),
    Token(TokenType::Comma),
    Token(TokenType::LeftParen),
    Token(TokenType::Number, "1"),
    Token(TokenType::Comma),
    Token(TokenType::Str, "Stalker"),
    Token(TokenType::Comma),
    Token(TokenType::Number, "1"),
    Token(TokenType::Comma),
    Token(TokenType::Number, "1"),
    Token(TokenType::Comma),
    Token(TokenType::Number, "1979"),
    Token(TokenType::Comma),
    Token(TokenType::Number, "8.2"),
    Token(TokenType::RightParen),
    Token(TokenType::Comma),
  }
}

} // namespace shaun::sql