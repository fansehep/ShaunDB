#include "src/sql/keyword.h"

#include <string.h>

namespace shaun {

namespace sql {

void to_upper(std::string *res) {
  for (uint64_t i = 0; i < res->size(); i++) {
    (*res)[i] = toupper((*res)[i]);
  }
}

std::string keyword_to_str(int k) {
  if (static_cast<uint64_t>(k) >=
      (sizeof(KeywordString) / sizeof(std::string))) {
    return std::string("");
  }
  return KeywordString[k];
}

void KeywordMap::init() {
  for (unsigned long i = 0; i < sizeof(KeywordString) / sizeof(std::string);
       i++) {
    key_word_map.emplace(KeywordString[i], static_cast<Keyword>(i));
  }
}

auto KeywordMap::get_type(const std::string &str) -> Keyword {
  // 全部大写
  std::string upper_str = str;
  to_upper(&upper_str);
  auto find_iter = key_word_map.find(upper_str);
  if (find_iter == key_word_map.end()) {
    return Keyword::UserIdent;
  }
  return find_iter->second;
}

auto KeywordMap::get_type(const std::string_view &str) -> Keyword {
  std::string upper_str(str);
  to_upper(&upper_str);
  auto find_iter = key_word_map.find(upper_str);
  if (find_iter == key_word_map.end()) {
    return Keyword::UserIdent;
  }
  return find_iter->second;
}

}  // namespace sql

}  // namespace shaun