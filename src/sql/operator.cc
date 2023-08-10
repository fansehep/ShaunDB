#include "src/sql/operator.h"

#include "src/sql/keyword.h"
#include "src/sql/operation.h"
#include "src/sql/token.h"

namespace shaun::sql {

auto InfixOperator::build(Expression* left, Expression* rig) -> Expression* {
  switch (this->t) {
    case Type::Add:
      return AddOper(left, rig).into_expr();
    case Type::And:
      return AndOper(left, rig).into_expr();
    case Type::Divide:
      return DivideOper(left, rig).into_expr();
    case Type::Equal:
      return EqualOper(left, rig).into_expr();
    case Type::Exponentiate:
      return ExponentiateOper(left, rig).into_expr();
    case Type::GreaterThan:
      return GreaterThanOper(left, rig).into_expr();
    case Type::GreaterThanOrEqual:
      return GreaterThanOrEqualOper(left, rig).into_expr();
    case Type::LessThan:
      return LessThanOper(left, rig).into_expr();
    case Type::LessThanOrEqual:
      return LessThanOrEqualOper(left, rig).into_expr();
    case Type::Like:
      return LikeOper(left, rig).into_expr();
    case Type::Modulo:
      return ModuloOper(left, rig).into_expr();
    case Type::Multiply:
      return MultiplyOper(left, rig).into_expr();
    case Type::NotEqual:
      return NotEqualOper(left, rig).into_expr();
    case Type::Or:
      return OrOper(left, rig).into_expr();
    case Type::Subtract:
      return SubtractOper(left, rig).into_expr();
    default:
      abort();
  }
}

auto InfixOperator::make_infix_oper(const Token& t) -> InfixOperator {
  switch (t.token_type()) {
    case TokenType::Asterisk: {
      return InfixOperator(Multiply);
    }
    case TokenType::Caret: {
      return InfixOperator(Exponentiate);
    }
    case TokenType::Equal: {
      return InfixOperator(Equal);
    }
    case TokenType::GreaterThan: {
      return InfixOperator(GreaterThan);
    }
    case TokenType::GreaterThanOrEqual: {
      return InfixOperator(GreaterThanOrEqual);
    }
    case TokenType::KeyWord: {
      switch (t.keyword_type()) {
        case Keyword::And: {
          return InfixOperator(And);
        }
        case Keyword::Like: {
          return InfixOperator(Like);
        }
        case Keyword::Or: {
          return InfixOperator(Or);
        }
        default: {
          // TODO: fix abort
          // abort();
          return InfixOperator(None);
        }
      }
    }
    case TokenType::LessThan: {
      return InfixOperator(LessThan);
    }
    case TokenType::LessThanOrEqual:
      return InfixOperator(LessThanOrEqual);
    case TokenType::Minus:
      return InfixOperator(Subtract);
    case TokenType::NotEqual:
      return InfixOperator(NotEqual);
    case TokenType::Percent:
      return InfixOperator(Modulo);
    case TokenType::Plus:
      return InfixOperator(Add);
    case TokenType::Slash:
      return InfixOperator(Divide);
    default: {
      return InfixOperator(None);
    }
  }
}

}  // namespace shaun::sql