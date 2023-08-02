#include "src/sql/datatype.h"


namespace shaun::sql {


    const char* DataType::to_string() const {
        switch (this->type_) {
            case Type::Boolean:
                return "Boolean";
            case Type::Integer:
                return "Integer";
            case Type::Float:
                return "Float";
            case Type::String:
                return "String";
            default:
                return "Unknown";
        }
    }




}