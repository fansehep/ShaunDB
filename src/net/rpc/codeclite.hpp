#ifndef SRC_NET_RPC_CODECLITE_H_
#define SRC_NET_RPC_CODECLITE_H_

#include <event2/buffer.h>
#include <zlib.h>

#include <memory>
#include <type_traits>

#include "src/base/log/logbuffer.hpp"
#include "src/base/log/logging.hpp"
#include "src/base/noncopyable.hpp"
#include "src/net/connection.hpp"
#include "src/net/net_server.hpp"

using ::std::placeholders::_1;
using ::std::placeholders::_2;
using ::std::placeholders::_3;

namespace google {

namespace protobuf {

class Message;

}

}  // namespace google

using ::fver::base::NonCopyable;
using ::fver::base::log::Buffer;

namespace fver {

namespace rpc {

using MessagePtr = std::shared_ptr<google::protobuf::Message>;

class ProtobufCodeLite : public NonCopyable {
 public:
  constexpr static int kHeaderLength = sizeof(int32_t);
  constexpr static int kCheckSumLength = sizeof(int32_t);
  constexpr static int kMaxMessageLength = 64 * 1024 * 1024;

  enum ErrorCode {
    kNoError = 0,
    kInvalidLength = 1,
    kCheckSumError = 2,
    kInvalidNameLength = 3,
    kUnknownMessageType = 4,
    kParseError = 5,
  };

  using RawMessageCallback = std::function<bool(net::ConnPtr, int32_t)>;
  using ProtobufMessageCallback =
      std::function<void(net::ConnPtr, const MessagePtr&)>;
  using ErrorCallback = std::function<void(net::ConnPtr, ErrorCode)>;

  ProtobufCodeLite(const ::google::protobuf::Message* proto_type,
                   const char* tag_arg,
                   const ProtobufMessageCallback& message_cb,
                   const RawMessageCallback& raw_cb = {},
                   const ErrorCallback& error_cb = defaultErrorCallback)
      : proto_type_(proto_type),
        tag_(tag_arg),
        message_cb_(message_cb),
        rawmessage_cb_(raw_cb),
        error_cb_(error_cb),
        kMinMessage_len(tag_.length() + kCheckSumLength) {
    LOG_INFO("ProtobufCodeLite construct!");
  }

  virtual ~ProtobufCodeLite() {}

  const std::string& tag();

  void Send(net::ConnPtr, const ::google::protobuf::Message& message);

  void onMessage(net::ConnPtr conn);

  bool parseFromBuffer(const unsigned char* buf, const size_t buf_len,
                       google::protobuf::Message* message);

  int serializeToBuffer(const google::protobuf::Message& message,
                        struct evbuffer* buf);

  static const std::string& errorcode_tostring(ErrorCode error_code);

  ErrorCode parse(net::ConnPtr conn, size_t len,
                  ::google::protobuf::Message* message);

  int fillEmptyBuffer(struct evbuffer* buf,
                      const google::protobuf::Message& message);

  static int32_t checkSum(struct evbuffer* buf, int len);

  static bool validateCheckSum(const Bytef* buf, int len);

  static int32_t asInt32(const char* buf);

  static void defaultErrorCallback(net::ConnPtr, ErrorCode);

 private:
  const ::google::protobuf::Message* proto_type_;
  const std::string tag_;
  ProtobufMessageCallback message_cb_;
  RawMessageCallback rawmessage_cb_;
  ErrorCallback error_cb_;
  const uint32_t kMinMessage_len;
};

template <typename msg, const char* tag, typename CODEC = ProtobufCodeLite>
class ProtobufCodeLiteT {
  static_assert(std::is_base_of_v<ProtobufCodeLite, CODEC>,
                "shold extend protobufcodelite");

 public:
  using ConcreateMessagePtr = std::shared_ptr<msg>;
  using ProtobufMessageCallback =
      std::function<void(net::ConnPtr, const ConcreateMessagePtr&)>;
  using RawMessageCallback = ProtobufCodeLite::RawMessageCallback;
  using ErrorCallback = ProtobufCodeLite::ErrorCallback;

  explicit ProtobufCodeLiteT(
      const ProtobufMessageCallback& mes_cb,
      const RawMessageCallback& raw_cb = {},
      const ErrorCallback& error_cb = ProtobufCodeLite::defaultErrorCallback)
      : message_cb_(mes_cb),
        codec_(&msg::default_instance, tag,
               std::bind(&ProtobufCodeLiteT::onRPCMessage, this, _1, _2),
               raw_cb, error_cb) {}

  const std::string& getTag() const { return codec_.tag(); }

  void Send(net::ConnPtr conn, const msg& mes) { codec_.send(conn, mes); }

  void onMessage(net::ConnPtr conn) { codec_.onMessage(conn); }

  void onRPCMessage(net::ConnPtr conn, const MessagePtr& mes);

  void fillEmptyBuffer(struct evbuffer* buf,
                       const google::protobuf::Message& mes) {
    codec_.fillEmptyBuffer(buf, mes);
  }

 private:
  ProtobufMessageCallback message_cb_;
  CODEC codec_;
};

}  // namespace rpc

}  // namespace fver
#endif