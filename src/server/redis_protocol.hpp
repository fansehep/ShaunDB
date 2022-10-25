#ifndef SRC_SERVER_REDIS_PROTOCOL_H_
#define SRC_SERVER_REDIS_PROTOCOL_H_

#include <fmt/format.h>

#include <cstring>
#include <string>
#include <string_view>
#include <variant>

namespace fver {

namespace protocol {

enum ContextType {
  kPing,
  kGet,
  kSet,
  kDel,
};

enum RedisReply {
  kStatusReply,
  kErrorReply,
  kIntegerReply,
  kBulkReply,
  kMultiBulkReply,
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

struct DelContext {
  std::string_view key_view;
  std::string_view value_view;
};

using Context_vec =
    std::variant<PingContext, GetContext, SetContext, DelContext>;

// set mykey myvalue
// fmt::print():
//    *3
//    $3
//    SET
//    $5
//    mykey
//    $7
//    myvalue
// real : *3\r\n$3\r\nSET\r\n$5\r\nmykey\r\n$7\r\nmyvalue\r\n
//
std::pair<Context_vec, ContextType> parseRedisProtocol(const char* data,
                                                       const size_t size) {
  std::pair<Context_vec, ContextType> result;
  auto arg_view = std::string_view(data, size);
  if (arg_view == kPingStr) {
    result.second = ContextType::kPing;
    result.first.emplace<PingContext>();
    return result;
  }
  int i = 0;
  int j = 0;
  
  int CommandN = 0;
  
}

}  // namespace protocol

}  // namespace fver

#endif