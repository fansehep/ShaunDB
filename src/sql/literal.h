#pragma once

#include <string>
#include <stdint.h>


namespace shaun::sql {


    struct Literal {
        Literal() = default;
        ~Literal() = default;
        int type;
    };

    struct NullLiteral : public Literal {

    };

    struct BooleanLiteral : public Literal {
        bool ue_;
    };

    struct IntegerLiteral : public Literal {
        int64_t value_;
    };

    struct FloatLiteral : public Literal {
        double value_;
    };

    struct StringLiteral : public Literal {
        std::string str_;
    };




}