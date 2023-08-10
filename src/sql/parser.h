#pragma once

#include "src/sql/lexer.h"
#include "src/sql/statement.h"

namespace shaun::sql {

class Parser {
 public:
  auto parse_statement() -> std::optional<Statement *>;

  explicit Parser(const char *source) : lexer_(source) {}

  explicit Parser(const std::string &source) : lexer_(source) {}

 private:
  auto next_token_is(Keyword k) -> bool;

  auto _parse_transaction() -> std::optional<Statement *>;

  auto _next_token() { next_token_ = lexer_.next_token(); }

  auto _parse_ddl() -> std::optional<Statement *>;

  auto _parse_ddl_create_table() -> std::optional<Statement *>;

  auto _parse_ddl_drop_table() -> std::optional<Statement *>;

  auto _parse_ddl_columns() -> std::variant<Column, std::string>;

  auto _parse_delete_stmt() -> std::optional<Statement *>;

  auto _parse_insert_stmt() -> std::optional<Statement *>;

  auto _parse_update_stmt() -> std::optional<Statement *>;
  //
  auto _parse_clause_where() -> std::optional<Expression *>;

  auto _parse_select_stmt() -> std::optional<Statement *>;

  auto _parse_clause_select()
      -> std::vector<std::pair<Expression *, std::string>>;

  auto _parse_expresion(int min_prec) -> std::optional<Expression *>;

  auto _parse_explain_stmt() -> std::optional<Statement *>;

  template <typename Oper>
  auto _parse_next_if_operator(int min_prec) -> std::optional<Oper>;

  Token current_token_;
  Token next_token_;

  Lexer lexer_;
};

}  // namespace shaun::sql