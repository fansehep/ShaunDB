#include "src/sql/lexer.h"

#include "gtest/gtest.h"
#include "src/log/slog.h"
#include "src/sql/keyword.h"
#include "src/sql/token.h"

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
  const char *sql =
      "CREATE TABLE movie ("
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
  const char *sql2 =
      "INSERT INTO movie "
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
      "(12, 'Eternal Sunshine of the Spotless Mind', 5, 3, 2004, 8.3);";
  lexer.update(sql3);
  res = {
      Token(TokenType::KeyWord, Keyword::Insert),
      Token(TokenType::KeyWord, Keyword::Into),
      Token(TokenType::Ident, "movies"),
      Token(TokenType::KeyWord, Keyword::Values),
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
      Token(TokenType::Number, "12"),
      Token(TokenType::Comma),
      Token(TokenType::Str, "Eternal Sunshine of the Spotless Mind"),
      Token(TokenType::Comma),
      Token(TokenType::Number, "5"),
      Token(TokenType::Comma),
      Token(TokenType::Number, "3"),
      Token(TokenType::Comma),
      Token(TokenType::Number, "2004"),
      Token(TokenType::Comma),
      Token(TokenType::Number, "8.3"),
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
}

TEST_F(LexerTest, BasicTest3) {
  const char *sql = "DroP Table Studios;";
  Lexer lexer(sql);
  std::vector<Token> res = {
      Token(TokenType::KeyWord, Keyword::Drop),
      Token(TokenType::KeyWord, Keyword::Table),
      Token(TokenType::Ident, "Studios"),
      Token(TokenType::Semicolon),
      Token(TokenType::Eof),
  };
  int i = 0;
  for (auto &it : res) {
    auto t = lexer.next_token();
    if (t != it) {
      Warn("idx: {} expected {} but get: {}", i, it, t);
    }
    ASSERT_EQ(t, it);
    i++;
  }
  const char *sql1 = "DELETE FROM studios WHERE id = 1;";
  lexer.update(sql1);
  res = {
      Token(TokenType::KeyWord, Keyword::Delete),
      Token(TokenType::KeyWord, Keyword::From),
      Token(TokenType::Ident, "studios"),
      Token(TokenType::KeyWord, Keyword::Where),
      Token(TokenType::Ident, "id"),
      Token(TokenType::Equal),
      Token(TokenType::Number, "1"),
      Token(TokenType::Semicolon),
      Token(TokenType::Eof),
  };
  i = 0;
  for (auto &it : res) {
    auto t = lexer.next_token();
    if (t != it) {
      Warn("idx: {} expected {} but get: {}", i, it, t);
    }
    ASSERT_EQ(t, it);
    i++;
  }
  const char *sql2 = "UPDATE moveies Set id = 1;";
  lexer.update(sql2);
  res = {
      Token(TokenType::KeyWord, Keyword::Update),
      Token(TokenType::Ident, "moveies"),
      Token(TokenType::KeyWord, Keyword::Set),
      Token(TokenType::Ident, "id"),
      Token(TokenType::Equal, "="),
      Token(TokenType::Number, "1"),
      Token(TokenType::Semicolon),
      Token(TokenType::Eof),
  };
  i = 0;
  for (auto &it : res) {
    auto t = lexer.next_token();
    if (t != it) {
      Warn("idx: {} expected {} but get: {}", i, it, t);
    }
    ASSERT_EQ(t, it);
    i++;
  }
}

TEST_F(LexerTest, OperatorTest) {
  const char *sql = "SELECT 3.14 * 2.718;";
  Lexer lexer(sql);
  std::vector<Token> res = {
      Token(TokenType::KeyWord, Keyword::Select),
      Token(TokenType::Number, "3.14"),
      Token(TokenType::Asterisk),
      Token(TokenType::Number, "2.718"),
      Token(TokenType::Semicolon, ";"),
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
  const char *sql1 = "SELECT 1.0 / 0.0;";
  lexer.update(sql1);
  res = {
      Token(TokenType::KeyWord, Keyword::Select),
      Token(TokenType::Number, "1.0"),
      Token(TokenType::Slash),
      Token(TokenType::Number, "0.0"),
      Token(TokenType::Semicolon, ";"),
      Token(TokenType::Eof),
  };
  i = 0;
  for (auto &it : res) {
    auto t = lexer.next_token();
    if (t != it) {
      Warn("idx: {} expected: {} but get: {}", i, it, t);
    }
    ASSERT_EQ(t, it);
    i++;
  }
  const char *sql2 = "SELECT 1 ^ 8 / INFINITY, 8 ^ 10, INFINITY / INFINITY;";
  lexer.update(sql2);
  res = {
      Token(TokenType::KeyWord, Keyword::Select),
      Token(TokenType::Number, "1"),
      Token(TokenType::Caret),
      Token(TokenType::Number, "8"),
      Token(TokenType::Slash, "/"),
      Token(TokenType::KeyWord, Keyword::Infinity),
      Token(TokenType::Comma),
      Token(TokenType::Number, "8"),
      Token(TokenType::Caret),
      Token(TokenType::Number, "10"),
      Token(TokenType::Comma),
      Token(TokenType::KeyWord, Keyword::Infinity),
      Token(TokenType::Slash),
      Token(TokenType::KeyWord, Keyword::Infinity),
      Token(TokenType::Semicolon),
      Token(TokenType::Eof),
  };
  i = 0;
  for (auto &it : res) {
    auto t = lexer.next_token();
    if (t != it) {
      Warn("idx: {} expected: {} but get: {}", i, it, t);
    }
    ASSERT_EQ(t, it);
    i++;
  }
  const char *sql3 = "SELECT NOT TRUE, NOT FALSE, NOT NULL;";
  lexer.update(sql3);
  res = {
      Token(TokenType::KeyWord, Keyword::Select),
      Token(TokenType::KeyWord, Keyword::Not),
      Token(TokenType::KeyWord, Keyword::True),
      Token(TokenType::Comma),
      Token(TokenType::KeyWord, Keyword::Not),
      Token(TokenType::KeyWord, Keyword::False),
      Token(TokenType::Comma),
      Token(TokenType::KeyWord, Keyword::Not),
      Token(TokenType::KeyWord, Keyword::Null),
      Token(TokenType::Semicolon),
      Token(TokenType::Eof),
  };
  i = 0;
  for (auto &it : res) {
    auto t = lexer.next_token();
    if (t != it) {
      Warn("idx: {} expected: {} but get: {}", i, it, t);
    }
    ASSERT_EQ(t, it);
    i++;
  }
}

TEST_F(LexerTest, JoinTokenTest) {
  const char *sql =
      "SELECT   m.id, m.title, g.name AS genre, m.released, s.name AS studio\n"
      "       FROM     movies m JOIN genres g ON m.genre_id = g.id,\n"
      "                studios s JOIN movies good ON good.studio_id = s.id AND "
      "good.rating >= 8\n"
      "       WHERE    m.studio_id = s.id AND m.released >= 2000 AND g.id = 1\n"
      "       ORDER BY m.title ASC;";
  Lexer lexer(sql);
  std::vector<Token> res = {
      Token(TokenType::KeyWord, Keyword::Select),
      Token(TokenType::Ident, "m"),
      Token(TokenType::Period),
      Token(TokenType::Ident, "id"),
      Token(TokenType::Comma),
      Token(TokenType::Ident, "m"),
      Token(TokenType::Period),
      Token(TokenType::Ident, "title"),
      Token(TokenType::Comma),
      Token(TokenType::Ident, "g"),
      Token(TokenType::Period),
      Token(TokenType::Ident, "name"),
      Token(TokenType::KeyWord, Keyword::As),
      Token(TokenType::Ident, "genre"),
      Token(TokenType::Comma),
      Token(TokenType::Ident, "m"),
      Token(TokenType::Period),
      Token(TokenType::Ident, "released"),
      Token(TokenType::Comma),
      Token(TokenType::Ident, "s"),
      Token(TokenType::Period),
      Token(TokenType::Ident, "name"),
      Token(TokenType::KeyWord, Keyword::As),
      Token(TokenType::Ident, "studio"),
      Token(TokenType::KeyWord, Keyword::From),
      Token(TokenType::Ident, "movies"),
      Token(TokenType::Ident, "m"),
      Token(TokenType::KeyWord, Keyword::Join),
      Token(TokenType::Ident, "genres"),
      Token(TokenType::Ident, "g"),
      Token(TokenType::KeyWord, Keyword::On),
      Token(TokenType::Ident, "m"),
      Token(TokenType::Period),
      Token(TokenType::Ident, "genre_id"),
      Token(TokenType::Equal),
      Token(TokenType::Ident, "g"),
      Token(TokenType::Period),
      Token(TokenType::Ident, "id"),
      Token(TokenType::Comma),
      Token(TokenType::Ident, "studios"),
      Token(TokenType::Ident, "s"),
      Token(TokenType::KeyWord, Keyword::Join),
      Token(TokenType::Ident, "movies"),
      Token(TokenType::Ident, "good"),
      Token(TokenType::KeyWord, Keyword::On),
      Token(TokenType::Ident, "good"),
      Token(TokenType::Period),
      Token(TokenType::Ident, "studio_id"),
      Token(TokenType::Equal),
      Token(TokenType::Ident, "s"),
      Token(TokenType::Period),
      Token(TokenType::Ident, "id"),
      Token(TokenType::KeyWord, Keyword::And),
      Token(TokenType::Ident, "good"),
      Token(TokenType::Period),
      Token(TokenType::Ident, "rating"),
      Token(TokenType::GreaterThanOrEqual),
      Token(TokenType::Number, "8"),
      Token(TokenType::KeyWord, Keyword::Where),
      Token(TokenType::Ident, "m"),
      Token(TokenType::Period),
      Token(TokenType::Ident, "studio_id"),
      Token(TokenType::Equal),
      Token(TokenType::Ident, "s"),
      Token(TokenType::Period),
      Token(TokenType::Ident, "id"),
      Token(TokenType::KeyWord, Keyword::And),
      Token(TokenType::Ident, "m"),
      Token(TokenType::Period),
      Token(TokenType::Ident, "released"),
      Token(TokenType::GreaterThanOrEqual),
      Token(TokenType::Number, "2000"),
      Token(TokenType::KeyWord, Keyword::And),
      Token(TokenType::Ident, "g"),
      Token(TokenType::Period),
      Token(TokenType::Ident, "id"),
      Token(TokenType::Equal),
      Token(TokenType::Number, "1"),
      Token(TokenType::KeyWord, Keyword::Order),
      Token(TokenType::KeyWord, Keyword::By),
      Token(TokenType::Ident, "m"),
      Token(TokenType::Period),
      Token(TokenType::Ident, "title"),
      Token(TokenType::KeyWord, Keyword::Asc),
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
  const char *sql2 = "BEGIN;";
  lexer.update(sql2);
  res = {
      Token(TokenType::KeyWord, Keyword::Begin),
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
  const char *sql3 = "ROLLBACK; COMMIT; BEGIN READ ONLY AS OF SYSTEM TIME 172;";
  lexer.update(sql3);
  res = {
      Token(TokenType::KeyWord, Keyword::Rollback),
      Token(TokenType::Semicolon),
      Token(TokenType::KeyWord, Keyword::Commit),
      Token(TokenType::Semicolon),
      Token(TokenType::KeyWord, Keyword::Begin),
      Token(TokenType::KeyWord, Keyword::Read),
      Token(TokenType::KeyWord, Keyword::Only),
      Token(TokenType::KeyWord, Keyword::As),
      Token(TokenType::KeyWord, Keyword::Of),
      Token(TokenType::KeyWord, Keyword::System),
      Token(TokenType::KeyWord, Keyword::Time),
      Token(TokenType::Number, "172"),
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
}

}  // namespace shaun::sql
