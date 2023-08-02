#pragma once

#include <string>
#include "src/sql/datatype.h"

namespace shaun::sql {

    class Expression;


    struct Column {
        std::string name;
        DataType data_type;
        bool is_primary_key;
        std::optional<bool> nullable;
        std::optional<Expression*> exp;
        bool is_unique;
        bool has_index;
        std::optional<std::string> referce;

    };



}