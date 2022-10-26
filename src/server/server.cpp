#include "src/server/server.hpp"

#include "src/db/request.hpp"
#include "src/db/status.hpp"
#include "src/net/connection.hpp"
#include "src/server/redis_protocol.hpp"

namespace fver {

namespace server {

bool Server::Init(const ServerConfig& config) {
  
}

int Server::readHD(char* buf, size_t size, Connection* conn) {
  auto read_result = redis::parseRedisProtocol(buf, size);
  // 解析失败, 直接返回
  if (false == redis::check(&read_result)) {
    if (read_result.size() >= 2) {
      auto error_reply =
          fmt::format("(error) ERR unknow command '{}'", read_result[1]);
      conn->Send(error_reply.c_str(), error_reply.size());
    } else {
      conn->Send(redis::kErrorUnknowCommand.c_str(),
                 redis::kErrorUnknowCommand.size());
    }
    return 1;
  }
  // PING
  if (*read_result.begin() == redis::kPingStr) {
    LOG_INFO("conn ip: {} port: {} ping server", conn->getPeerIP(),
             conn->getPeerIP());
    conn->Send(redis::kPongStr.c_str(), redis::kPongStr.size());
    return -1;
    // set 请求
  } else if (*read_result.begin() == redis::kSetStr) {
    LOG_INFO("conn ip: {} port: {} set key: {} value: {}", conn->getPeerIP(),
             conn->getPeerPort(), read_result[1], read_result[2]);
    auto set_context =
        std::make_shared<db::SetContext>(read_result[1], read_result[2]);
    db_.Set(set_context);
    if (set_context->code.getCode() == db::StatusCode::kOk) {
      conn->Send(redis::kOKReply.c_str(), redis::kOKReply.size());
    }
    return -1;
    // get 请求
  } else if (*read_result.begin() == redis::kGetStr) {
    auto get_context = std::make_shared<db::GetContext>(read_result[1]);
    db_.Get(get_context);
    LOG_INFO("conn ip: {} port: {} get key: {} value: {}", conn->getPeerIP(),
             conn->getPeerPort(), get_context->key, get_context->value);
    if (get_context->code.getCode() == db::StatusCode::kOk) {
      auto get_reply = fmt::format("${}\r\n{}\r\n", get_context->key.size(),
                                   get_context->key);
      conn->Send(get_reply.c_str(), get_reply.size());
      return -1;
    } else {
      return -1;
    }
  }
  return -1;
}

}  // namespace server

}  // namespace fver