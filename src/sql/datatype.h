#pragma once

#include <string>

#include "fmt/format.h"

namespace shaun::sql {

class DataType {
 public:
  enum Type {
    Boolean,
    Integer,
    Float,
    String,
  };
  int type_;

  const char* to_string() const;

  void set_type(int t) { type_ = t; }
};

struct Value {
  DataType type;
};

struct NullValue : public Value {};

struct BooleanValue : public Value {
  bool ue;
};

struct IntegerValue : public Value {
  int64_t val_;
};

struct FloatValue : public Value {
  double val_;
};

struct StringValue : public Value {
  std::string val_;
};

enum OrderType {
  Ascending,
  Descending,
};

}  // namespace shaun::sql

template <>
struct fmt::formatter<shaun::sql::DataType> {
  constexpr auto parse(const format_parse_context& c) { return c.begin(); }
  template <typename Context>
  constexpr auto format(const shaun::sql::DataType& d, Context& c) const {
    return fmt::format_to(c.out(), "DataType: {}", d.to_string());
  }
};