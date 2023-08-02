#pragma once

#include <optional>
#include <string>
//#include "src/sql/literal.h"
//#include "src/sql/operation.h"

namespace shaun::sql {


    class Literal;
    class Operation;

    class Expression {
    public:
        Expression() = default;
        ~Expression() = default;
        int type;
    };

    class FiledExpr : public Expression{
    public:
        std::optional<std::string> filed_name_;
        std::string value_;
    };

    class ColumnExpr : public Expression {
    public:
        uint32_t size_;
    };

    class LiteralExpr : public Expression{
    public:
        Literal* liter_;
    };

    class FunctionExpr : public Expression {
    public:
        std::string function_name_;
        std::vector<Expression*> args_;
    };

    class OperationExpr : public Expression {
    public:
        Operation* op_;
    };



}