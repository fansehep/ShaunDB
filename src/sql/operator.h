#pragma once

#include "src/common/helper.h"
#include "src/sql/expression.h"

namespace shaun::sql {

class Token;
class Parser;

class INTERFACE Operator {
 public:
  virtual Operator* make(const Token& t) = 0;

  virtual Operator* augement(Parser* p) = 0;

  virtual uint8_t assoc() = 0;

  virtual uint8_t prec() = 0;
  enum Type {
    
  };
  
  int type;
};

struct InfixOperator : public Operator{
  enum Type {
    Add,
    And,
    Divide,
    Equal,
    Exponentiate,
    GreaterThan,
    GreaterThanOrEqual,
    LessThan,
    LessThanOrEqual,
    Like,
    Modulo,
    Multiply,
    NotEqual,
    Or,
    Subtract,
    None,
  };
  int t;

  InfixOperator(Type type) : t(type) {}

  auto build(Expression* left, Expression* rig) -> Expression*;

  static auto make_infix_oper(const Token& t) -> InfixOperator;


  // impl Base Operator
  
};

}  // namespace shaun::sql