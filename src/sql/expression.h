#pragma once

#include <optional>
#include <string>

namespace shaun::sql {

class Literal;
class Operation;

class Expression {
 public:
  enum Type {
    Filed,
    Column,
    Literal,
    Function,
    Operation,
  };
  Expression() = default;
  ~Expression() = default;
  Expression(int t) : type(t) {}
  int type;
};

class FiledExpr : public Expression {
 public:
  FiledExpr() : Expression(Type::Filed) {}
  std::optional<std::string> filed_name_;
  std::string value_;
};

class ColumnExpr : public Expression {
 public:
  ColumnExpr() : Expression(Type::Column) {}
  uint32_t size_;

};

class LiteralExpr : public Expression {
 public:
  LiteralExpr() : Expression(Type::Literal) {}
  Literal* liter_;
};

class FunctionExpr : public Expression {
 public:
  FunctionExpr() : Expression(Type::Function) {}
  std::string function_name_;
  std::vector<Expression*> args_;
};

class OperationExpr : public Expression {
 public:
  OperationExpr() : Expression(Type::Operation) {}
  Operation* op_;
};

}  // namespace shaun::sql