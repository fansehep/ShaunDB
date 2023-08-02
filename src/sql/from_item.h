#pragma once

#include <string>

namespace shaun::sql {



    class Expression;

    enum JoinType {
        Cross,
        Inner,
        Left,
        Right,
    };


    class FromItem {
    public:
        FromItem() = default;
        ~FromItem() = default;
        int type;
    };

    class TableFromItem : public FromItem {
    public:
        std::string name_;
        std::optional<std::string> alias_;
    };

    class JoinFromitem : public FromItem {
    public:
        FromItem* left_;
        FromItem* right_;
        JoinType join_type_;
        std::optional<Expression*> predicate_;
    };




}