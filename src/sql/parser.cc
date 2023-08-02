#pragma once

#include "src/sql/parser.h"
#include "src/log/slog.h"

namespace shaun::sql {



    auto Parser::parse_statement() -> std::optional<Statement*> {
        auto tok = lexer_.next_token();
        // is_end 判断 Token::Eof,
        if (tok.is_end()) {
            return std::make_optional<Statement*>();
        }
        current_token_ = tok;
         if (tok.is_keyword()) {
             switch (tok.key_word.value()) {
                 case Keyword::Begin:
                 case Keyword::Commit:
                 case Keyword::Rollback:
                    return _parse_transaction();
                 case Keyword::Create:
                 case Keyword::Drop:
                     return _parse_ddl();

                 case Keyword::Delete:
                     return _parse_delete_stmt();
                 case Keyword::Insert:
                     return _parse_insert_stmt();
                 case Keyword::Select:
                     return _parse_select_stmt();
                 case Keyword::Update:
                     return _parse_update_stmt();
                 case Keyword::Explain:
                     return _parse_explain();
                 default:
                     return std::make_optional<ErrStmt*>(new ErrStmt(fmt::format("unexpected token: {}", current_token_)));
             }
         }
         return std::make_optional<Statement*>();
    }


    auto Parser::_parse_transaction() -> std::optional<Statement*> {
        // BEGIN TRANSACTION
        this->next_token_ = lexer_.next_token();
        switch (current_token_.keyword_type()) {
            case Keyword::Begin:
                // begin transaction read only
                // begin transaction read write
                bool is_read_only = false;
                std::optional<int64_t> version = std::make_optional<int64_t>();
                //
                if (next_token_.keyword_type() == Keyword::Transaction) {
                    next_token_ = lexer_.next_token();
                }
                if (next_token_.keyword_type() == Keyword::Read) {
                    next_token_ = lexer_.next_token();
                    if (next_token_.keyword_type() == Keyword::Only) {
                        is_read_only = true;
                        next_token_ = lexer_.next_token();
                    } else if (next_token_.keyword_type() == Keyword::Write) {
                        next_token_ = lexer_.next_token();
                        is_read_only = false;
                    } else {
                        return std::make_optional<Statement*>(new ErrStmt(fmt::format("unexpected token: {}", next_token_)));
                    }
                }
                if (next_token_.keyword_type() == Keyword::As) {
                    _next_token();
                    if (next_token_.keyword_type() != Keyword::Of) {
                        return std::make_optional<Statement*>(new ErrStmt(fmt::format("unexpected token: {}", next_token_)));
                    }
                    _next_token();
                    if (next_token_.keyword_type() != Keyword::System) {
                        return std::make_optional<Statement*>(new ErrStmt(fmt::format("unexpected token: {}", next_token_)));
                    }
                    _next_token();
                    if (next_token_.keyword_type() != Keyword::Time) {
                        return std::make_optional<Statement*>(new ErrStmt(fmt::format("unexpected token: {}", next_token_)));
                    }
                    _next_token();
                    if (next_token_.token_type() == TokenType::Number) {
                        try {
                            version = std::stoll(next_token_.value);
                        } catch (const std::exception& e) {
                            Dwarn("parse Number error: ", e.what());
                            return std::make_optional<Statement*>(new ErrStmt(fmt::format("unexpected token: {}", next_token_)));
                        }

                    } else {
                        return std::make_optional<Statement*>(new ErrStmt(fmt::format("unexpected token: {}", next_token_)));
                    }
                }
                return std::make_optional<Statement*>(new BeginStmt(is_read_only, version));
                break;
            case Keyword::Commit:
                auto commit_stmt = new (std::nothrow) CommitStmt();
                return std::make_optional<Statement*>(commit_stmt);
            case Keyword::Rollback:
                auto rollback_stmt = new (std::nothrow) RollbackStmt();
                return std::make_optional<Statement*>(rollback_stmt);
            default:
                return std::make_optional<Statement*>(new ErrStmt(fmt::format("unexpected token: {}", next_token_)));
        }
    }


    auto Parser::_parse_ddl() -> std::optional<Statement *> {
        this->next_token_ = lexer_.next_token();
        switch (current_token_.keyword_type()) {
            case Keyword::Create:
                if (next_token_.keyword_type() == Keyword::Table) {
                    return _parse_ddl_create_table();
                } else {
                    return std::make_optional<Statement*>(new ErrStmt(fmt::format("unexpected token: {}", next_token_)));
                }
                break;
            case Keyword::Drop:
                if (next_token_.keyword_type() == Keyword::Table) {
                    return _parse_ddl_drop_table();
                } else {
                    return std::make_optional<Statement*>(new ErrStmt(fmt::format("unexpected token: {}", next_token_)));
                }
                break;
            default:
                return std::make_optional<Statement*>(new ErrStmt(fmt::format("unexpected token: {}", next_token_)));
        }
    }

    auto Parser::_parse_ddl_create_table() -> std::optional<Statement *> {
        _next_token();
        if (next_token_.token_type() != TokenType::Str) {
            return std::make_optional<Statement*>(new ErrStmt(fmt::format("unexpected token: {}", next_token_)));
        }
        auto table_name = next_token_.value;
        _next_token();
        // create table table_name (xxx_type xxx_attr xxx_attr,
        //                          xxx_type xxx_attr xxx_attr,
        //                          );
        if (next_token_.token_type() != TokenType::LeftParen) {
            return std::make_optional<Statement*>(new ErrStmt(fmt::format("unexpected token: {}", next_token_)));
        }
        auto create_stmt = new CreateTableStmt();
        while (true) {
            auto result = _parse_ddl_columns();
            if (result.index() == 1) {
                return std::make_optional<Statement*>(new ErrStmt(std::get<std::string>(result)));
            }
            create_stmt->columns_.emplace_back(std::get<Column>(result));
            _next_token();
            // ,
            if (next_token_.token_type() != TokenType::Comma) {
                break;
            }
        }
        if (next_token_.token_type() != TokenType::RightParen) {
            return std::make_optional<Statement*>(new ErrStmt(fmt::format("unexpected token: {}", next_token_)));
        }
        return std::make_optional<Statement*>(create_stmt);
    }

    auto Parser::_parse_ddl_columns() -> std::variant<Column, std::string> {
        _next_token();
        std::variant<Column, std::string> result;
        if (next_token_.token_type() != TokenType::Str) {
            result = fmt::format("unexpected token: {}", next_token_);
            return result;
        }
        _next_token();
        Column c;
        switch (next_token_.keyword_type()) {
            case Keyword::Char:
            case Keyword::Varchar:
            case Keyword::Text:
            case Keyword::String:
                c.data_type.set_type(DataType::Type::String);
                break;
            case Keyword::Boolean:
            case Keyword::Bool:
                c.data_type.set_type(DataType::Type::Boolean);
                break;
            case Keyword::Double:
            case Keyword::Float:
                c.data_type.set_type(DataType::Type::Float);
                break;
            case Keyword::Int:
            case Keyword::Integer:
                c.data_type.set_type(DataType::Type::Integer);
                break;
            default:
                result = fmt::format("unexpected token: {}", next_token_);
                return result;
        }

        _next_token();
        while (next_token_.token_type() == TokenType::KeyWord) {
            switch (next_token_.keyword_type()) {
                case Keyword::Primary:
                    _next_token();
                    if (next_token_.keyword_type() != Keyword::Key) {
                        result = fmt::format("unexpected token: {}", next_token_);
                        return result;
                    }
                    c.is_primary_key = true;
                    break;
                case Keyword::Null:
                    if (c.nullable.has_value() && c.nullable.value() == false) {
                        result = fmt::format("unexpected token: {}", next_token_);
                        return result;
                    }
                    c.nullable = true;
                    break;
                case Keyword::Not:
                    _next_token();
                    if (next_token_.keyword_type() != Keyword::Null) {
                        result = fmt::format("unexpected token: {}", next_token_);
                        return result;
                    }
                    if (c.nullable.has_value() && c.nullable.value() == true) {
                        result = fmt::format("unexpected token: {}", next_token_);
                        return result;
                    }
                    c.nullable = false;
                case Keyword::Default:
                    //
                    break;
                case Keyword::Unique:
                    c.is_unique = true;
                    break;
                case Keyword::Index:
                    c.has_index = true;
                case Keyword::References:
                    _next_token();
                    if (next_token_.token_type() != TokenType::Ident) {
                        result = fmt::format("unexpected token: {}", next_token_);
                        return result;
                    }
                    c.referce = next_token_.value;
                default:
                    result = fmt::format("unexpected token: {}", next_token_);
                    return result;
            }
            _next_token();
        }
        return c;
    }

    auto Parser::_parse_ddl_drop_table() -> std::optional<Statement*> {
        _next_token();
        if (next_token_.token_type() == TokenType::Ident && next_token_.keyword_type() == Keyword::UserIdent) {
            auto drop_stmt = new (std::nothrow) DropTableStmt(next_token_.value);
            return std::make_optional<Statement*>(drop_stmt);
        }
        return std::make_optional<Statement*>(new ErrStmt(fmt::format("unexpected token: {}", next_token_)));
    }

    auto Parser::_parse_delete_stmt() -> std::optional<Statement *> {
        // DELETE FROM Table_Name
        _next_token();
        if (next_token_.keyword_type() != Keyword::From) {
            return std::make_optional<Statement*>(new ErrStmt(fmt::format("unexpected token: {}", next_token_)));
        }
        _next_token();
        if (next_token_.keyword_type() != Keyword::UserIdent) {
            return std::make_optional<Statement*>(new ErrStmt(fmt::format("unexpected token: {}", next_token_)));
        }
        auto delete_stmt = new DeleteStmt(next_token_.value);
        return std::make_optional<Statement*>(delete_stmt);
    }

    auto Parser::_parse_insert_stmt() -> std::optional<Statement *>  {

    }

    auto Parser::_parse_clause_where() -> std::optional<Expression*> {
        _next_token();
        if (next_token_.keyword_type() != Keyword::Where) {
            return std::make_optional<Expression*>();
        }
        return _parse_expresion();
    }

    auto Parser::_parse_expresion() -> std::optional<Expression *> {

    }

    auto Parser::_parse_select_stmt() -> std::optional<Statement*> {

    }

}