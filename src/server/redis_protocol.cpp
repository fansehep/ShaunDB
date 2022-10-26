#include "src/server/redis_protocol.hpp"

#include <charconv>
#include <deque>

namespace fver {

namespace redis {

std::deque<std::string_view> parseRedisProtocol(const char* data,
                                                 const size_t size) {
  std::deque<std::string_view> result;
  auto arg_view = std::string_view(data, size);
  if (arg_view == kPingStr) {
    result.push_back(kPingStr);
    return result;
  }
  int i = 0;
  for (; i < size; i++) {
    switch (data[i]) {
      case '+': {
        // simple string 单行字符串,
        auto simple_string_index = arg_view.find("\r\n", i);
        if (simple_string_index == std::string_view::npos) {
          result.push_back(kErrorStr);
          return result;
        }
        auto value = std::string_view(data + i + 1, simple_string_index - i);
#ifdef FVER_REDIS_PROTOCOL_DEBUG
        LOG_INFO("simple string value = {}", value);
#endif
        result.push_back(value);
        i = (simple_string_index + 1);
        break;
      }
      case '-': {
        // '-' ERROR 错误
        auto simple_string_index = arg_view.find("\r\n", i);
        if (simple_string_index == std::string_view::npos) {
          result.push_back(kErrorStr);
          return result;
        }
        auto value = std::string_view(data + i + 1, simple_string_index - i);
#ifdef FVER_REDIS_PROTOCOL_DEBUG
        LOG_INFO("error value = {}", value);
#endif
        result.push_back(value);
        i = (simple_string_index + 1);
        break;
      }
      case ':': {
        // ':' Integer
        auto simple_string_index = arg_view.find("\r\n", i);
        if (simple_string_index == std::string_view::npos) {
          result.push_back(kErrorStr);
          return result;
        }
        auto value = std::string_view(data + i + 1, simple_string_index - i);
#ifdef FVER_REDIS_PROTOCOL_DEBUG
        LOG_INFO("integer value = {}", value);
#endif
        result.push_back(value);
        i = (simple_string_index + 1);
        break;
      }
      case '$': {
        // '$': 后面字符串的长度
        auto simple_string_index = arg_view.find("\r\n", i);
        if (simple_string_index == std::string_view::npos) {
          result.push_back(kErrorStr);
          return result;
        }
        auto value = std::string_view(data + i + 1, simple_string_index - i);
#ifdef FVER_REDIS_PROTOCOL_DEBUG
        LOG_INFO("string_len number = {}", value);
#endif
        int number = 0;
        std::from_chars(value.data(), value.data() + value.size(), number);
        i = simple_string_index;
        auto result_view = std::string_view(data + i + 2, number);
        result.push_back(result_view);
        i += number;
#ifdef FVER_REDIS_PROTOCOL_DEBUG
        LOG_INFO("string = {}", value);
#endif
        break;
      }
      case '*': {
        // '*': RESP 数组的长度
        auto simple_string_index = arg_view.find("\r\n");
        if (simple_string_index == std::string_view::npos) {
          result.push_back(kErrorStr);
          return result;
        }
        auto number_view =
            std::string_view(data + i + 1, simple_string_index - i - 1);
#ifdef FVER_REDIS_PROTOCOL_DEBUG
        LOG_INFO("simple_string_index = {} number_view = {}",
                 simple_string_index, number_view);
#endif
        result.push_back(number_view);
        i = (simple_string_index + 1);
        break;
      }
      default: {
        continue;
      }
    }
  }
  return result;
}

bool check(std::deque<std::string_view>* vec_view) {
  if (vec_view->empty()) {
    return false;
  }
  int command_n = 0;
  auto first_view = vec_view->at(0);
  if (first_view == kPingStr) {
    return true;
  }
  if (first_view[0] >= '0' && first_view[0] <= '9') {
    std::from_chars(first_view.data(), first_view.data() + first_view.size(),
                    command_n);
    if (command_n != vec_view->size()) {
      return false;
    }
    vec_view->pop_front();
    return true;
  }
  return true;
}

}  // namespace redis
}  // namespace fver