#include "src/server/server.hpp"

#include "src/base/log/logging.hpp"
#include "src/db/request.hpp"
#include "src/db/status.hpp"
#include "src/net/connection.hpp"
#include "src/server/redis_protocol.hpp"

namespace fver {

namespace server {

bool Server::Init(const struct ServerConfig& conf) {
  if (conf.logLev != -1) {
    fver::base::log::Init(conf.logpath, conf.logLev, conf.isSync,
                          conf.log_name);
  }
  return server_.Init(conf.listen_port, std::bind(&Server::writeHD, this, _1),
                      std::bind(&Server::closeHD, this, _1),
                      std::bind(&Server::timeoutHD, this, _1),
                      std::bind(&Server::readHD, this, _1));
}

void Server::Run() { server_.Run(); }

int Server::readHD(const std::shared_ptr<Connection>& conn) {
  auto size = conn->moveEvReadBuffer(buf_.bufptr_ + buf_.offset_, buf_.buflen_);
  buf_.offset_ += size;
  auto read_str_view = std::string_view(buf_.bufptr_, buf_.offset_);
  LOG_TRACE("buf.offset = {} buf.context = {}", buf_.offset_, read_str_view);
  auto buf = buf_.bufptr_;

  if (buf[size - 1] == '\n') {
    size -= 1;
  }

  if (read_str_view == redis::kCommandDocs ||
      read_str_view == redis::kCommandDocs_2) {
    conn->Send(redis::kOKStr.c_str(), redis::kOKStr.length());
    buf_.offset_ = 0;
    return -1;
  }

  LOG_INFO("conn ip: {} port: {} send {}", conn->getPeerIP(),
           conn->getPeerPort(), read_str_view);
  auto read_result = redis::parseRedisProtocol(buf, size);
  // 解析失败, 直接返回
  if (false == redis::check(&read_result)) {
    LOG_INFO("conn ip: {} port: {} parse error!", conn->getPeerIP(),
             conn->getPeerPort());
    if (read_result.size() >= 2) {
      auto error_reply =
          fmt::format("(error) ERR unknow command '{}'\n", read_result[1]);
      conn->Send(error_reply.c_str(), error_reply.size());
    } else {
      conn->Send(redis::kErrorUnknowCommand.c_str(),
                 redis::kErrorUnknowCommand.size());
    }
    buf_.offset_ = 0;
    return -1;
  }
  // PING
  if (*read_result.begin() == redis::kPingStr) {
    LOG_INFO("conn ip: {} port: {} ping server", conn->getPeerIP(),
             conn->getPeerIP());
    conn->Send(redis::kPongStr.c_str(), redis::kPongStr.size());
    buf_.offset_ = 0;
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
    buf_.offset_ = 0;
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
      buf_.offset_ = 0;
      return -1;
    } else {
      buf_.offset_ = 0;
      return -1;
    }
  } else {
    std::string result;
    for(auto& str : read_result) {
      result += " ";
      result += str;
    }
    LOG_DEBUG("Parse error! {}", result);
  }
  buf_.offset_ = 0;
  return -1;
}

int Server::writeHD(const std::shared_ptr<Connection>& conn) { return 1; }

int Server::closeHD(const std::shared_ptr<Connection>& conn) {
  LOG_INFO("conn ip: {} port: {} close", conn->getPeerIP(),
           conn->getPeerPort());
  return 1;
}

int Server::timeoutHD(const std::shared_ptr<Connection>& conn) {
  LOG_INFO("conn ip: {} port: {} timeout", conn->getPeerIP(),
           conn->getPeerPort());
  return 1;
}

void Server::Stop() {}

}  // namespace server

}  // namespace fver