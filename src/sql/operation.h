#pragma once



#include "src/sql/expression.h"

namespace shaun::sql {


    struct Operation {
        Operation() = default;
        ~Operation() = default;
        int type;
    };

    struct AndOper : public Operation {
        Expression* left_exp_;
        Expression* right_exp_;
    };

    struct NotOper : public Operation {
        Expression* exp_;
    };

    struct OrOper : public Operation {
        Expression* left_exp_;
        Expression* right_exp_;
    };

    struct EqualOper : public Operation {
        Expression* left_exp_;
        Expression* right_exp_;
    };

    struct GreaterThanOper : public Operation {
        Expression* left_exp_;
        Expression* right_exp_;
    };

    struct GreaterThanOrEqualOper : public Operation {
        Expression* left_exp_;
        Expression* right_exp_;
    };

    struct IsNullOpr : public Operation {
        Expression* exp_;
    };

    struct LessThanOper : public Operation {
        Expression* left_exp_;
        Expression* right_exp_;
    };

    struct LessThanOrEqualOper : public Operation {
        Expression* left_exp_;
        Expression* right_exp_;
    };

    struct NotEqualOper : public Operation {
        Expression* left_exp_;
        Expression* right_exp_;
    };

    struct AddOper : public Operation {
        Expression* left_exp_;
        Expression* right_exp_;
    };

    struct AssertOper : public Operation {
        Expression* exp_;
    };

    struct DivideOper : public Operation {
        Expression* left_exp_;
        Expression* right_exp_;
    };

    struct ExponentiateOper : public Operation {
        Expression* left_exp_;
        Expression* right_exp_;
    };

    struct FactorialOper : public Operation {
        Expression* exp_;
    };

    struct ModuloOper : public Operation {
        Expression* left_exp_;
        Expression* right_exp_;
    };

    struct MoltiplyOper : public Operation {
        Expression* left_exp_;
        Expression* right_exp_;
    };

    struct NegateOper : public Operation {
        Expression* exp_;
    };

    struct SubtractOper : public Operation {
        Expression* left_exp_;
        Expression* right_exp_;
    };

    struct LikeOper : public Operation {
        Expression* left_exp_;
        Expression* right_exp_;
    };

}