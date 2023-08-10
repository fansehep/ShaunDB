#pragma once

#include <map>
#include <optional>
#include <string>
#include <vector>

#include "src/sql/column.h"

namespace shaun::sql {

class Expression;
class Operation;

class Statement {
 protected:
  enum StmtType {
    ShowTable,
    Begin,
    Commit,
    Rollback,
    Explain,
    Delete,
    Insert,
    Update,
    Select,
    CreateTable,
    DropTable,
    Err,
  };

 public:
  Statement() = default;
  ~Statement() = default;
  Statement(StmtType t) : type(t) {}
  int type;
};

class ShowTableStmt : public Statement {
 public:
  ShowTableStmt() : Statement(StmtType::ShowTable) {}
  // todo: may need add database;
};

class BeginStmt : public Statement {
 public:
  BeginStmt() : Statement(StmtType::Begin) {}
  bool is_readonly_;
  std::optional<int64_t> version_;
  BeginStmt(bool ue, const std::optional<int64_t>& v)
      : is_readonly_(ue), version_(v) {}
};

class CommitStmt : public Statement {
 public:
  CommitStmt() : Statement(StmtType::Commit) {}
};

class RollbackStmt : public Statement {
 public:
  RollbackStmt() : Statement(StmtType::Rollback) {}
};

class ExplainStmt : public Statement {
 public:
  ExplainStmt() : Statement(StmtType::Explain) {}
  Statement* stmt_;
  ExplainStmt(Statement* s) : Statement(StmtType::Explain), stmt_(s) {}
};

class DeleteStmt : public Statement {
 public:
  DeleteStmt() : Statement(StmtType::Delete) {}
  DeleteStmt(const std::string& name) : table_name_(name) {}
  std::string table_name_;
  Expression* exp_;
};

class InsertStmt : public Statement {
 public:
  InsertStmt() : Statement(StmtType::Insert) {}
  std::string table_name_;
  std::optional<std::vector<std::string>> columns_;
  std::vector<std::vector<Expression*>> exps_;
};

class UpdateStmt : public Statement {
 public:
  UpdateStmt() : Statement(StmtType::Update) {}
  std::string table_name_;
  std::map<std::string, Expression*> sets_;
  Expression* expr;
};

class SelectStmt : public Statement {
 public:
  SelectStmt() : Statement(StmtType::Select) {}
  std::vector<std::pair<Expression*, std::string>> selects_;
};

struct CreateTableStmt : public Statement {
 public:
  CreateTableStmt() : Statement(StmtType::CreateTable) {}
  std::string table_name_;
  std::vector<Column> columns_;
};

class ErrStmt : public Statement {
 public:
  ErrStmt() : Statement(StmtType::Err) {}
  std::string error_msg_;
  std::optional<Expression*> expr_;
  explicit ErrStmt(const std::string& e) : error_msg_(e) {}
};

class DropTableStmt : public Statement {
 public:
  DropTableStmt() : Statement(StmtType::DropTable) {}
  std::string drop_table_name_;
  explicit DropTableStmt(const std::string& name) : drop_table_name_(name) {}
};


}  // namespace shaun::sql