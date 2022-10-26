#ifndef SRC_SERVER_REDIS_PROTOCOL_H_
#define SRC_SERVER_REDIS_PROTOCOL_H_

#include <fmt/format.h>

#include <charconv>
#include <cstring>
#include <deque>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace fver {

namespace redis {

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

const std::string kErrorUnknowCommand = "(error) ERR unknow command";
const std::string kPingStr = "PING";
const std::string kPongStr = "+PONG";
const std::string kOKStr = "+OK";
const std::string kSetStr = "SET";
const std::string kGetStr = "GET";
const std::string kDelStr = "DEL";
const std::string kErrorStr = "Error";

const std::string kOKReply = "+OK\r\n";

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
/*
 * parse the client to server redis command.
 * if some error occured, the last string_view is kErrorStr;
 *
 */
std::deque<std::string_view> parseRedisProtocol(const char* data,
                                                const size_t size);

// remove some useless string_view
bool check(std::deque<std::string_view>* vec_view);
}  // namespace redis

}  // namespace fver

#endif