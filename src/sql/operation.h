#pragma once

#include "src/sql/expression.h"
#include "src/sql/lexer.h"

namespace shaun::sql {

struct Operation {
  enum Type {
    And,
    Not,
    Or,
    Equal,
    GreaterThan,
    GreaterThanOrEqual,
    IsNull,
    LessThan,
    LessThanOrEqual,
    NotEqual,
    Add,
    Assert,
    Divide,
    Exponentiate,
    Factorial,
    Modulo,
    Multiply,
    Negate,
    Subtract,
    Like,
  };
  Operation(Type t) : type(t) {}

  Operation() = default;
  ~Operation() = default;
  auto into_expr() -> Expression* { return (Expression*)(this); }
  int type;
};

struct AndOper : public Operation {
  AndOper() : Operation(Type::And) {}
  Expression* left_exp_;
  Expression* right_exp_;
  explicit AndOper(Expression* l, Expression* r)
      : Operation(Type::And), left_exp_(l), right_exp_(r) {}
};

struct NotOper : public Operation {
  NotOper() : Operation(Type::Not) {}
  Expression* exp_;
  explicit NotOper(Expression* e) : Operation(Type::Not), exp_(e) {}
};

struct OrOper : public Operation {
  OrOper() : Operation(Type::Or) {}
  Expression* left_exp_;
  Expression* right_exp_;
  explicit OrOper(Expression* l, Expression* r)
      : Operation(Type::Or), left_exp_(l), right_exp_(r) {}
};

struct EqualOper : public Operation {
  EqualOper() : Operation(Type::Equal) {}
  Expression* left_exp_;
  Expression* right_exp_;
  explicit EqualOper(Expression* l, Expression* r)
      : Operation(Type::Equal), left_exp_(l), right_exp_(r) {}
};

struct GreaterThanOper : public Operation {
  GreaterThanOper() : Operation(Type::GreaterThan) {}
  Expression* left_exp_;
  Expression* right_exp_;
  explicit GreaterThanOper(Expression* l, Expression* r)
      : Operation(Type::GreaterThan), left_exp_(l), right_exp_(r) {}
};

struct GreaterThanOrEqualOper : public Operation {
  GreaterThanOrEqualOper() : Operation(Type::GreaterThanOrEqual) {}
  Expression* left_exp_;
  Expression* right_exp_;
  explicit GreaterThanOrEqualOper(Expression* l, Expression* r)
      : Operation(Type::GreaterThanOrEqual), left_exp_(l), right_exp_(r) {}
};

struct IsNullOper : public Operation {
  IsNullOper() : Operation(Type::IsNull) {}
  Expression* exp_;
  explicit IsNullOper(Expression* e) : Operation(Type::IsNull), exp_(e) {}
};

struct LessThanOper : public Operation {
  LessThanOper() : Operation(Type::LessThan) {}
  Expression* left_exp_;
  Expression* right_exp_;
  explicit LessThanOper(Expression* l, Expression* r)
      : Operation(Type::LessThan), left_exp_(l), right_exp_(r) {}
};

struct LessThanOrEqualOper : public Operation {
  LessThanOrEqualOper() : Operation(Type::LessThanOrEqual) {}
  Expression* left_exp_;
  Expression* right_exp_;
  explicit LessThanOrEqualOper(Expression* l, Expression* r)
      : Operation(Type::LessThanOrEqual), left_exp_(l), right_exp_(r) {}
};

struct NotEqualOper : public Operation {
  NotEqualOper() : Operation(Type::NotEqual) {}
  Expression* left_exp_;
  Expression* right_exp_;
  explicit NotEqualOper(Expression* l, Expression* r)
      : Operation(Type::NotEqual), left_exp_(l), right_exp_(r) {}
};

struct AddOper : public Operation {
  AddOper() : Operation(Type::Add) {}
  Expression* left_exp_;
  Expression* right_exp_;
  explicit AddOper(Expression* l, Expression* r)
      : Operation(Type::Add), left_exp_(l), right_exp_(r) {}
};

struct AssertOper : public Operation {
  AssertOper() : Operation(Type::Assert) {}
  Expression* exp_;
  AssertOper(Expression* e) : Operation(Type::Assert), exp_(e) {}
};

struct DivideOper : public Operation {
  DivideOper() : Operation(Type::Divide) {}
  Expression* left_exp_;
  Expression* right_exp_;
  explicit DivideOper(Expression* l, Expression* r)
      : Operation(Type::Divide), left_exp_(l), right_exp_(r) {}
};

struct ExponentiateOper : public Operation {
  ExponentiateOper() : Operation(Type::Exponentiate) {}
  Expression* left_exp_;
  Expression* right_exp_;
  explicit ExponentiateOper(Expression* l, Expression* r)
      : Operation(Type::Exponentiate), left_exp_(l), right_exp_(r) {}
};

struct FactorialOper : public Operation {
  FactorialOper() : Operation(Type::Factorial) {}
  Expression* exp_;
  explicit FactorialOper(Expression* e) : Operation(Type::Factorial), exp_(e) {}
};

struct ModuloOper : public Operation {
  ModuloOper() : Operation(Type::Modulo) {}
  Expression* left_exp_;
  Expression* right_exp_;
  explicit ModuloOper(Expression* l, Expression* r)
      : Operation(Type::Modulo), left_exp_(l), right_exp_(r) {}
};

struct MultiplyOper : public Operation {
  MultiplyOper() : Operation(Type::Multiply) {}
  Expression* left_exp_;
  Expression* right_exp_;
  explicit MultiplyOper(Expression* l, Expression* r)
      : Operation(Type::Multiply), left_exp_(l), right_exp_(r) {}
};

struct NegateOper : public Operation {
  NegateOper() : Operation(Type::Negate) {}
  Expression* exp_;
  explicit NegateOper(Expression* e) : Operation(Type::Negate), exp_(e) {}
};

struct SubtractOper : public Operation {
  SubtractOper() : Operation(Type::Subtract) {}
  Expression* left_exp_;
  Expression* right_exp_;
  explicit SubtractOper(Expression* l, Expression* r)
      : Operation(Type::Subtract), left_exp_(l), right_exp_(r) {}
};

struct LikeOper : public Operation {
  LikeOper() : Operation(Type::Like) {}
  Expression* left_exp_;
  Expression* right_exp_;
  explicit LikeOper(Expression* l, Expression* r)
      : Operation(Type::Like), left_exp_(l), right_exp_(r) {}
};

}  // namespace shaun::sql