#ifndef SRC_SERVER_REDIS_PROTOCOL_H_
#define SRC_SERVER_REDIS_PROTOCOL_H_

#include <fmt/format.h>

#include <charconv>
#include <cstring>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "src/base/log/logging.hpp"

namespace fver {

namespace protocol {

enum ContextType {
  kPing,
  kGet,
  kSet,
  kDel,
  kError,
};

const char RedisReplyLev[] = {
    '+',  // 状态回复
    '-',  // 错误回复
    ':',  // 整数回复
    '$',  // 批量回复
    '*',  // 多条批量回复
};

const std::string kPingStr = "PING";
const std::string kPongStr = "+PONG";
const std::string kOKStr = "+OK";
const std::string kSetStr = "SET";
const std::string kGetStr = "GET";
const std::string kDelStr = "DEL";
const std::string kErrorStr = "Error";

// Ping 命令直接回复即可
struct PingContext {
  std::string_view reply;
  PingContext() : reply(kPongStr) {}
};

struct GetContext {
  std::string_view key_view;
  std::string_view value_view;
};

struct SetContext {
  std::string_view key_view;
  std::string_view value_view;
};

struct ExistsContext {
  std::string_view key_view;
  bool is_exits;
};

struct DelContext {
  std::string_view key_view;
  std::string_view value_view;
};

// client: PING
// reply: +PONG
// client: set mykey myvalue
// reply: real : *3\r\n$3\r\nSET\r\n$5\r\nmykey\r\n$7\r\nmyvalue\r\n
// client: get my
// reply:
//
std::vector<std::string_view> parseRedisProtocol(const char* data,
                                                 const size_t size) {
  std::vector<std::string_view> result;
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
        i = simple_string_index;
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
        i = simple_string_index;
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
        i = simple_string_index;
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
        auto number_view = std::string_view(data + i, simple_string_index - i);
#ifdef FVER_REDIS_PROTOCOL_DEBUG
        LOG_INFO("number_view = {}", number_view);
#endif
        result.push_back(number_view);
        i += simple_string_index + 2;
        break;
      }
      default: {
      }
    }
    return result;
  }
}

}  // namespace protocol

}  // namespace fver

#endif