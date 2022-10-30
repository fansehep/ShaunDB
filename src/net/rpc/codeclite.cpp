#include "src/net/rpc/codeclite.hpp"

#include <endian.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <zlib.h>

#include "src/base/log/logging.hpp"
#include "src/net/connection.hpp"
#include "src/net/net_server.hpp"
#include "src/net/rpc/google-inl.hpp"

namespace {
int ProtobufVersionCheck() {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  return 0;
}
int __attribute__((unused)) dummy = ProtobufVersionCheck();
}  // namespace

namespace fver {

namespace rpc {

void ProtobufCodeLite::Send(net::ConnPtr conn,
                            const ::google::protobuf::Message &message) {
  auto ev_buf = evbuffer_new();
  if (fillEmptyBuffer(ev_buf, message)) {
    LOG_ERROR("send error {}",
              evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
  }
  if (conn->addbufToWriteBuffer(ev_buf) != 0) {
    LOG_TRACE("send error: {}",
              evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
  }
  ::evbuffer_free(ev_buf);
}

int ProtobufCodeLite::fillEmptyBuffer(
    struct ::evbuffer *buf, const google::protobuf::Message &message) {
  int res = 0;
  if ((res = evbuffer_add(buf, tag_.c_str(), tag_.size())) < 0) {
    return res;
  }
  int byte_size = serializeToBuffer(message, buf);
  int32_t check_sum = checkSum(buf, byte_size + tag_.size());
  int len = htobe32(check_sum);
  if ((res = evbuffer_add(buf, &len, sizeof(len))) < 0) {
    return res;
  }
  len = ::htobe32(tag_.size() + byte_size + kCheckSumLength);
  if ((res = evbuffer_prepend(buf, &len, sizeof(len)))) {
    return res;
  }
  return 0;
}

int ProtobufCodeLite::serializeToBuffer(
    const google::protobuf::Message &message, struct ::evbuffer *buf) {
  GOOGLE_DCHECK(message.IsInitialized())
      << InitializationErrorMessage("serialize", message);
  struct evbuffer_iovec vec;
  uint32_t byte_size = message.ByteSizeLong();
  evbuffer_reserve_space(buf, byte_size + kCheckSumLength, &vec, 1);
  uint8_t *start = static_cast<uint8_t *>(vec.iov_base);
  uint8_t *end = message.SerializeWithCachedSizesToArray(start);
  vec.iov_len = end - start;
  if (vec.iov_len != byte_size) {
    ByteSizeConsistencyError(byte_size, message.ByteSizeLong(), (end - start));
  }
  evbuffer_commit_space(buf, &vec, 1);
  return vec.iov_len;
}

void ProtobufCodeLite::onMessage(net::ConnPtr conn) {
  LOG_INFO("received message len: {}", conn->getEvReadBufferLen());
  while (conn->getEvReadBufferLen() >=
         static_cast<uint32_t>(kMinMessage_len + kHeaderLength)) {
    int32_t len = 0;
    conn->moveEvReadBuffer(reinterpret_cast<char *>(&len), sizeof(len));
    len = asInt32(reinterpret_cast<char *>(&len));
    if (len > kMaxMessageLength ||
        static_cast<uint32_t>(len) < kMinMessage_len) {
      error_cb_(conn, kInvalidLength);
      break;
    } else if (conn->getEvReadBufferLen() >= (static_cast<uint32_t>(len))) {
      if (rawmessage_cb_ && !rawmessage_cb_(conn, len)) {
        evbuffer_drain(conn->getReadBuf(), len);
        continue;
      }
      MessagePtr message(proto_type_->New());
      auto error_code = parse(conn, len, message.get());
      if (error_code == kNoError) {
        message_cb_(conn, message);
        evbuffer_drain(conn->getReadBuf(), len);
      } else {
        error_cb_(conn, error_code);
        break;
      }
    } else {
      break;
    }
  }
}

bool ProtobufCodeLite::parseFromBuffer(const unsigned char *buf,
                                       const size_t buf_len,
                                       google::protobuf::Message *message) {
  return message->ParseFromArray(buf, buf_len);
}

namespace {
const std::string kNoErrorStr = "NoError";
const std::string kInvalidLengthStr = "InvalidLength";
const std::string kCheckSumErrorStr = "CheckSumError";
const std::string kInvalidNameLenStr = "InvalidNameLen";
const std::string kUnknownMessageTypeStr = "UnknownMessageType";
const std::string kParseErrorStr = "ParseError";
const std::string kUnknownErrorStr = "UnknownError";
}  // namespace

const std::string &ProtobufCodeLite::errorcode_tostring(ErrorCode errorCode) {
  switch (errorCode) {
    case kNoError:
      return kNoErrorStr;
    case kInvalidLength:
      return kInvalidLengthStr;
    case kCheckSumError:
      return kCheckSumErrorStr;
    case kInvalidNameLength:
      return kInvalidNameLenStr;
    case kUnknownMessageType:
      return kUnknownMessageTypeStr;
    case kParseError:
      return kParseErrorStr;
    default:
      return kUnknownErrorStr;
  }
}

void ProtobufCodeLite::defaultErrorCallback(net::ConnPtr conn,
                                            ErrorCode errorCode) {
  // LOG(DEBUF)<< "ProtobufCodecLite::defaultErrorCallback - " <<
  // errorCodeToString(errorCode);
  if (conn) {
    bufferevent_free(conn->getBufferEvent());
    auto pair = conn->getConnText();
    pair.first->removeConn(pair.second);
  }
}

int32_t ProtobufCodeLite::asInt32(const char *buf) {
  int32_t be32 = 0;
  ::memcpy(&be32, buf, sizeof(be32));
  return be32toh(be32);
}

int32_t ProtobufCodeLite::checkSum(struct evbuffer *buf, int len) {
  int dlen = 0;
  uLong adler = adler32(1L, Z_NULL, 0);
  int n = evbuffer_peek(buf, len, NULL, NULL, 0);
  evbuffer_iovec *vec = new evbuffer_iovec[n];
  int res = evbuffer_peek(buf, len, NULL, vec, n);
  assert(res == n);
  for (int i = 0; i < n; i++) {
    dlen += vec[i].iov_len;
    adler = adler32(adler, static_cast<const Bytef *>(vec[i].iov_base),
                    vec[i].iov_len);
  }
  delete[] vec;
  //  LOG(INFO) << "Len : "<< dlen << "checksum : "<<adler;

  return static_cast<int32_t>(adler);
}

bool ProtobufCodeLite::validateCheckSum(const Bytef *buf, int len) {
  int32_t expectedCheckSum =
      asInt32(reinterpret_cast<const char *>(buf) + len - kCheckSumLength);
  uLong adler = adler32(1L, Z_NULL, 0);
  adler = adler32(adler, (buf), len - kCheckSumLength);
  //  LOG(INFO) << "Len : "<< len-kChecksumLen << "checksum : "<<adler;

  return (static_cast<int32_t>(adler)) == (expectedCheckSum);
}

ProtobufCodeLite::ErrorCode ProtobufCodeLite::parse(
    net::ConnPtr conn, const size_t len, ::google::protobuf::Message *message) {
  ErrorCode error = kNoError;
  evbuffer *readBuffer = conn->getReadBuf();
  //  std::vector<char> tag(tag_.size());
  unsigned char *data = evbuffer_pullup(readBuffer, len);
  if (data == nullptr) {
    //
  }
  /*
    struct evbuffer_iovec vec ;
    res = evbuffer_peek(readBuffer, len, NULL, &vec, 1);
    assert( res == 1);
  */
  if (validateCheckSum(static_cast<const Bytef *>(data), len)) {
    if (memcmp(data, tag_.data(), tag_.size()) == 0) {
      // parse from buffer
      int32_t dataLen = len - kCheckSumLength - static_cast<int>(tag_.size());
      if (parseFromBuffer(data + tag_.size(), dataLen, message)) {
        error = kNoError;
      } else {
        error = kParseError;
      }
    } else {
      error = kUnknownMessageType;
    }
  } else {
    error = kCheckSumError;
  }

  return error;
}

}  // namespace rpc

}  // namespace fver