// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: rpc.proto

#ifndef GOOGLE_PROTOBUF_INCLUDED_rpc_2eproto
#define GOOGLE_PROTOBUF_INCLUDED_rpc_2eproto

#include <limits>
#include <string>

#include <google/protobuf/port_def.inc>
#if PROTOBUF_VERSION < 3019000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers. Please update
#error your headers.
#endif
#if 3019001 < PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers. Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/port_undef.inc>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_table_driven.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/metadata_lite.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/generated_enum_reflection.h>
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>
#define PROTOBUF_INTERNAL_EXPORT_rpc_2eproto
PROTOBUF_NAMESPACE_OPEN
namespace internal {
class AnyMetadata;
}  // namespace internal
PROTOBUF_NAMESPACE_CLOSE

// Internal implementation detail -- do not use these members.
struct TableStruct_rpc_2eproto {
  static const ::PROTOBUF_NAMESPACE_ID::internal::ParseTableField entries[]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::AuxiliaryParseTableField aux[]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::ParseTable schema[1]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::FieldMetadata field_metadata[];
  static const ::PROTOBUF_NAMESPACE_ID::internal::SerializationTable serialization_table[];
  static const uint32_t offsets[];
};
extern const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable descriptor_table_rpc_2eproto;
namespace frpc {
class RPCMessage;
struct RPCMessageDefaultTypeInternal;
extern RPCMessageDefaultTypeInternal _RPCMessage_default_instance_;
}  // namespace frpc
PROTOBUF_NAMESPACE_OPEN
template<> ::frpc::RPCMessage* Arena::CreateMaybeMessage<::frpc::RPCMessage>(Arena*);
PROTOBUF_NAMESPACE_CLOSE
namespace frpc {

enum MessageType : int {
  kUnknow = 0,
  kRequest = 1,
  kResponse = 2,
  kError = 3,
  MessageType_INT_MIN_SENTINEL_DO_NOT_USE_ = std::numeric_limits<int32_t>::min(),
  MessageType_INT_MAX_SENTINEL_DO_NOT_USE_ = std::numeric_limits<int32_t>::max()
};
bool MessageType_IsValid(int value);
constexpr MessageType MessageType_MIN = kUnknow;
constexpr MessageType MessageType_MAX = kError;
constexpr int MessageType_ARRAYSIZE = MessageType_MAX + 1;

const ::PROTOBUF_NAMESPACE_ID::EnumDescriptor* MessageType_descriptor();
template<typename T>
inline const std::string& MessageType_Name(T enum_t_value) {
  static_assert(::std::is_same<T, MessageType>::value ||
    ::std::is_integral<T>::value,
    "Incorrect type passed to function MessageType_Name.");
  return ::PROTOBUF_NAMESPACE_ID::internal::NameOfEnum(
    MessageType_descriptor(), enum_t_value);
}
inline bool MessageType_Parse(
    ::PROTOBUF_NAMESPACE_ID::ConstStringParam name, MessageType* value) {
  return ::PROTOBUF_NAMESPACE_ID::internal::ParseNamedEnum<MessageType>(
    MessageType_descriptor(), name, value);
}
enum ErrorCode : int {
  kOk = 0,
  kParseError = 1,
  kNoService = 2,
  kNoMethod = 3,
  ErrorCode_INT_MIN_SENTINEL_DO_NOT_USE_ = std::numeric_limits<int32_t>::min(),
  ErrorCode_INT_MAX_SENTINEL_DO_NOT_USE_ = std::numeric_limits<int32_t>::max()
};
bool ErrorCode_IsValid(int value);
constexpr ErrorCode ErrorCode_MIN = kOk;
constexpr ErrorCode ErrorCode_MAX = kNoMethod;
constexpr int ErrorCode_ARRAYSIZE = ErrorCode_MAX + 1;

const ::PROTOBUF_NAMESPACE_ID::EnumDescriptor* ErrorCode_descriptor();
template<typename T>
inline const std::string& ErrorCode_Name(T enum_t_value) {
  static_assert(::std::is_same<T, ErrorCode>::value ||
    ::std::is_integral<T>::value,
    "Incorrect type passed to function ErrorCode_Name.");
  return ::PROTOBUF_NAMESPACE_ID::internal::NameOfEnum(
    ErrorCode_descriptor(), enum_t_value);
}
inline bool ErrorCode_Parse(
    ::PROTOBUF_NAMESPACE_ID::ConstStringParam name, ErrorCode* value) {
  return ::PROTOBUF_NAMESPACE_ID::internal::ParseNamedEnum<ErrorCode>(
    ErrorCode_descriptor(), name, value);
}
// ===================================================================

class RPCMessage final :
    public ::PROTOBUF_NAMESPACE_ID::Message /* @@protoc_insertion_point(class_definition:frpc.RPCMessage) */ {
 public:
  inline RPCMessage() : RPCMessage(nullptr) {}
  ~RPCMessage() override;
  explicit constexpr RPCMessage(::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized);

  RPCMessage(const RPCMessage& from);
  RPCMessage(RPCMessage&& from) noexcept
    : RPCMessage() {
    *this = ::std::move(from);
  }

  inline RPCMessage& operator=(const RPCMessage& from) {
    CopyFrom(from);
    return *this;
  }
  inline RPCMessage& operator=(RPCMessage&& from) noexcept {
    if (this == &from) return *this;
    if (GetOwningArena() == from.GetOwningArena()
  #ifdef PROTOBUF_FORCE_COPY_IN_MOVE
        && GetOwningArena() != nullptr
  #endif  // !PROTOBUF_FORCE_COPY_IN_MOVE
    ) {
      InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }

  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* descriptor() {
    return GetDescriptor();
  }
  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* GetDescriptor() {
    return default_instance().GetMetadata().descriptor;
  }
  static const ::PROTOBUF_NAMESPACE_ID::Reflection* GetReflection() {
    return default_instance().GetMetadata().reflection;
  }
  static const RPCMessage& default_instance() {
    return *internal_default_instance();
  }
  static inline const RPCMessage* internal_default_instance() {
    return reinterpret_cast<const RPCMessage*>(
               &_RPCMessage_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    0;

  friend void swap(RPCMessage& a, RPCMessage& b) {
    a.Swap(&b);
  }
  inline void Swap(RPCMessage* other) {
    if (other == this) return;
  #ifdef PROTOBUF_FORCE_COPY_IN_SWAP
    if (GetOwningArena() != nullptr &&
        GetOwningArena() == other->GetOwningArena()) {
   #else  // PROTOBUF_FORCE_COPY_IN_SWAP
    if (GetOwningArena() == other->GetOwningArena()) {
  #endif  // !PROTOBUF_FORCE_COPY_IN_SWAP
      InternalSwap(other);
    } else {
      ::PROTOBUF_NAMESPACE_ID::internal::GenericSwap(this, other);
    }
  }
  void UnsafeArenaSwap(RPCMessage* other) {
    if (other == this) return;
    GOOGLE_DCHECK(GetOwningArena() == other->GetOwningArena());
    InternalSwap(other);
  }

  // implements Message ----------------------------------------------

  RPCMessage* New(::PROTOBUF_NAMESPACE_ID::Arena* arena = nullptr) const final {
    return CreateMaybeMessage<RPCMessage>(arena);
  }
  using ::PROTOBUF_NAMESPACE_ID::Message::CopyFrom;
  void CopyFrom(const RPCMessage& from);
  using ::PROTOBUF_NAMESPACE_ID::Message::MergeFrom;
  void MergeFrom(const RPCMessage& from);
  private:
  static void MergeImpl(::PROTOBUF_NAMESPACE_ID::Message* to, const ::PROTOBUF_NAMESPACE_ID::Message& from);
  public:
  PROTOBUF_ATTRIBUTE_REINITIALIZES void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  const char* _InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) final;
  uint8_t* _InternalSerialize(
      uint8_t* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const final;
  int GetCachedSize() const final { return _cached_size_.Get(); }

  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(RPCMessage* other);

  private:
  friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
  static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
    return "frpc.RPCMessage";
  }
  protected:
  explicit RPCMessage(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                       bool is_message_owned = false);
  private:
  static void ArenaDtor(void* object);
  inline void RegisterArenaDtor(::PROTOBUF_NAMESPACE_ID::Arena* arena);
  public:

  static const ClassData _class_data_;
  const ::PROTOBUF_NAMESPACE_ID::Message::ClassData*GetClassData() const final;

  ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadata() const final;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  enum : int {
    kServiceFieldNumber = 3,
    kMethodFieldNumber = 4,
    kRequestFieldNumber = 5,
    kResponseFieldNumber = 6,
    kIdFieldNumber = 2,
    kTypeFieldNumber = 1,
    kErrorCodeFieldNumber = 7,
  };
  // optional string service = 3;
  bool has_service() const;
  private:
  bool _internal_has_service() const;
  public:
  void clear_service();
  const std::string& service() const;
  template <typename ArgT0 = const std::string&, typename... ArgT>
  void set_service(ArgT0&& arg0, ArgT... args);
  std::string* mutable_service();
  PROTOBUF_NODISCARD std::string* release_service();
  void set_allocated_service(std::string* service);
  private:
  const std::string& _internal_service() const;
  inline PROTOBUF_ALWAYS_INLINE void _internal_set_service(const std::string& value);
  std::string* _internal_mutable_service();
  public:

  // optional string method = 4;
  bool has_method() const;
  private:
  bool _internal_has_method() const;
  public:
  void clear_method();
  const std::string& method() const;
  template <typename ArgT0 = const std::string&, typename... ArgT>
  void set_method(ArgT0&& arg0, ArgT... args);
  std::string* mutable_method();
  PROTOBUF_NODISCARD std::string* release_method();
  void set_allocated_method(std::string* method);
  private:
  const std::string& _internal_method() const;
  inline PROTOBUF_ALWAYS_INLINE void _internal_set_method(const std::string& value);
  std::string* _internal_mutable_method();
  public:

  // optional bytes request = 5;
  bool has_request() const;
  private:
  bool _internal_has_request() const;
  public:
  void clear_request();
  const std::string& request() const;
  template <typename ArgT0 = const std::string&, typename... ArgT>
  void set_request(ArgT0&& arg0, ArgT... args);
  std::string* mutable_request();
  PROTOBUF_NODISCARD std::string* release_request();
  void set_allocated_request(std::string* request);
  private:
  const std::string& _internal_request() const;
  inline PROTOBUF_ALWAYS_INLINE void _internal_set_request(const std::string& value);
  std::string* _internal_mutable_request();
  public:

  // optional bytes response = 6;
  bool has_response() const;
  private:
  bool _internal_has_response() const;
  public:
  void clear_response();
  const std::string& response() const;
  template <typename ArgT0 = const std::string&, typename... ArgT>
  void set_response(ArgT0&& arg0, ArgT... args);
  std::string* mutable_response();
  PROTOBUF_NODISCARD std::string* release_response();
  void set_allocated_response(std::string* response);
  private:
  const std::string& _internal_response() const;
  inline PROTOBUF_ALWAYS_INLINE void _internal_set_response(const std::string& value);
  std::string* _internal_mutable_response();
  public:

  // optional fixed64 id = 2;
  bool has_id() const;
  private:
  bool _internal_has_id() const;
  public:
  void clear_id();
  uint64_t id() const;
  void set_id(uint64_t value);
  private:
  uint64_t _internal_id() const;
  void _internal_set_id(uint64_t value);
  public:

  // optional .frpc.MessageType type = 1;
  bool has_type() const;
  private:
  bool _internal_has_type() const;
  public:
  void clear_type();
  ::frpc::MessageType type() const;
  void set_type(::frpc::MessageType value);
  private:
  ::frpc::MessageType _internal_type() const;
  void _internal_set_type(::frpc::MessageType value);
  public:

  // optional .frpc.ErrorCode error_code = 7;
  bool has_error_code() const;
  private:
  bool _internal_has_error_code() const;
  public:
  void clear_error_code();
  ::frpc::ErrorCode error_code() const;
  void set_error_code(::frpc::ErrorCode value);
  private:
  ::frpc::ErrorCode _internal_error_code() const;
  void _internal_set_error_code(::frpc::ErrorCode value);
  public:

  // @@protoc_insertion_point(class_scope:frpc.RPCMessage)
 private:
  class _Internal;

  template <typename T> friend class ::PROTOBUF_NAMESPACE_ID::Arena::InternalHelper;
  typedef void InternalArenaConstructable_;
  typedef void DestructorSkippable_;
  ::PROTOBUF_NAMESPACE_ID::internal::HasBits<1> _has_bits_;
  mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr service_;
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr method_;
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr request_;
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr response_;
  uint64_t id_;
  int type_;
  int error_code_;
  friend struct ::TableStruct_rpc_2eproto;
};
// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// RPCMessage

// optional .frpc.MessageType type = 1;
inline bool RPCMessage::_internal_has_type() const {
  bool value = (_has_bits_[0] & 0x00000020u) != 0;
  return value;
}
inline bool RPCMessage::has_type() const {
  return _internal_has_type();
}
inline void RPCMessage::clear_type() {
  type_ = 0;
  _has_bits_[0] &= ~0x00000020u;
}
inline ::frpc::MessageType RPCMessage::_internal_type() const {
  return static_cast< ::frpc::MessageType >(type_);
}
inline ::frpc::MessageType RPCMessage::type() const {
  // @@protoc_insertion_point(field_get:frpc.RPCMessage.type)
  return _internal_type();
}
inline void RPCMessage::_internal_set_type(::frpc::MessageType value) {
  _has_bits_[0] |= 0x00000020u;
  type_ = value;
}
inline void RPCMessage::set_type(::frpc::MessageType value) {
  _internal_set_type(value);
  // @@protoc_insertion_point(field_set:frpc.RPCMessage.type)
}

// optional fixed64 id = 2;
inline bool RPCMessage::_internal_has_id() const {
  bool value = (_has_bits_[0] & 0x00000010u) != 0;
  return value;
}
inline bool RPCMessage::has_id() const {
  return _internal_has_id();
}
inline void RPCMessage::clear_id() {
  id_ = uint64_t{0u};
  _has_bits_[0] &= ~0x00000010u;
}
inline uint64_t RPCMessage::_internal_id() const {
  return id_;
}
inline uint64_t RPCMessage::id() const {
  // @@protoc_insertion_point(field_get:frpc.RPCMessage.id)
  return _internal_id();
}
inline void RPCMessage::_internal_set_id(uint64_t value) {
  _has_bits_[0] |= 0x00000010u;
  id_ = value;
}
inline void RPCMessage::set_id(uint64_t value) {
  _internal_set_id(value);
  // @@protoc_insertion_point(field_set:frpc.RPCMessage.id)
}

// optional string service = 3;
inline bool RPCMessage::_internal_has_service() const {
  bool value = (_has_bits_[0] & 0x00000001u) != 0;
  return value;
}
inline bool RPCMessage::has_service() const {
  return _internal_has_service();
}
inline void RPCMessage::clear_service() {
  service_.ClearToEmpty();
  _has_bits_[0] &= ~0x00000001u;
}
inline const std::string& RPCMessage::service() const {
  // @@protoc_insertion_point(field_get:frpc.RPCMessage.service)
  return _internal_service();
}
template <typename ArgT0, typename... ArgT>
inline PROTOBUF_ALWAYS_INLINE
void RPCMessage::set_service(ArgT0&& arg0, ArgT... args) {
 _has_bits_[0] |= 0x00000001u;
 service_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, static_cast<ArgT0 &&>(arg0), args..., GetArenaForAllocation());
  // @@protoc_insertion_point(field_set:frpc.RPCMessage.service)
}
inline std::string* RPCMessage::mutable_service() {
  std::string* _s = _internal_mutable_service();
  // @@protoc_insertion_point(field_mutable:frpc.RPCMessage.service)
  return _s;
}
inline const std::string& RPCMessage::_internal_service() const {
  return service_.Get();
}
inline void RPCMessage::_internal_set_service(const std::string& value) {
  _has_bits_[0] |= 0x00000001u;
  service_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, value, GetArenaForAllocation());
}
inline std::string* RPCMessage::_internal_mutable_service() {
  _has_bits_[0] |= 0x00000001u;
  return service_.Mutable(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, GetArenaForAllocation());
}
inline std::string* RPCMessage::release_service() {
  // @@protoc_insertion_point(field_release:frpc.RPCMessage.service)
  if (!_internal_has_service()) {
    return nullptr;
  }
  _has_bits_[0] &= ~0x00000001u;
  auto* p = service_.ReleaseNonDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArenaForAllocation());
#ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
  if (service_.IsDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited())) {
    service_.Set(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), "", GetArenaForAllocation());
  }
#endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
  return p;
}
inline void RPCMessage::set_allocated_service(std::string* service) {
  if (service != nullptr) {
    _has_bits_[0] |= 0x00000001u;
  } else {
    _has_bits_[0] &= ~0x00000001u;
  }
  service_.SetAllocated(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), service,
      GetArenaForAllocation());
#ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
  if (service_.IsDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited())) {
    service_.Set(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), "", GetArenaForAllocation());
  }
#endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
  // @@protoc_insertion_point(field_set_allocated:frpc.RPCMessage.service)
}

// optional string method = 4;
inline bool RPCMessage::_internal_has_method() const {
  bool value = (_has_bits_[0] & 0x00000002u) != 0;
  return value;
}
inline bool RPCMessage::has_method() const {
  return _internal_has_method();
}
inline void RPCMessage::clear_method() {
  method_.ClearToEmpty();
  _has_bits_[0] &= ~0x00000002u;
}
inline const std::string& RPCMessage::method() const {
  // @@protoc_insertion_point(field_get:frpc.RPCMessage.method)
  return _internal_method();
}
template <typename ArgT0, typename... ArgT>
inline PROTOBUF_ALWAYS_INLINE
void RPCMessage::set_method(ArgT0&& arg0, ArgT... args) {
 _has_bits_[0] |= 0x00000002u;
 method_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, static_cast<ArgT0 &&>(arg0), args..., GetArenaForAllocation());
  // @@protoc_insertion_point(field_set:frpc.RPCMessage.method)
}
inline std::string* RPCMessage::mutable_method() {
  std::string* _s = _internal_mutable_method();
  // @@protoc_insertion_point(field_mutable:frpc.RPCMessage.method)
  return _s;
}
inline const std::string& RPCMessage::_internal_method() const {
  return method_.Get();
}
inline void RPCMessage::_internal_set_method(const std::string& value) {
  _has_bits_[0] |= 0x00000002u;
  method_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, value, GetArenaForAllocation());
}
inline std::string* RPCMessage::_internal_mutable_method() {
  _has_bits_[0] |= 0x00000002u;
  return method_.Mutable(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, GetArenaForAllocation());
}
inline std::string* RPCMessage::release_method() {
  // @@protoc_insertion_point(field_release:frpc.RPCMessage.method)
  if (!_internal_has_method()) {
    return nullptr;
  }
  _has_bits_[0] &= ~0x00000002u;
  auto* p = method_.ReleaseNonDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArenaForAllocation());
#ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
  if (method_.IsDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited())) {
    method_.Set(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), "", GetArenaForAllocation());
  }
#endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
  return p;
}
inline void RPCMessage::set_allocated_method(std::string* method) {
  if (method != nullptr) {
    _has_bits_[0] |= 0x00000002u;
  } else {
    _has_bits_[0] &= ~0x00000002u;
  }
  method_.SetAllocated(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), method,
      GetArenaForAllocation());
#ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
  if (method_.IsDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited())) {
    method_.Set(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), "", GetArenaForAllocation());
  }
#endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
  // @@protoc_insertion_point(field_set_allocated:frpc.RPCMessage.method)
}

// optional bytes request = 5;
inline bool RPCMessage::_internal_has_request() const {
  bool value = (_has_bits_[0] & 0x00000004u) != 0;
  return value;
}
inline bool RPCMessage::has_request() const {
  return _internal_has_request();
}
inline void RPCMessage::clear_request() {
  request_.ClearToEmpty();
  _has_bits_[0] &= ~0x00000004u;
}
inline const std::string& RPCMessage::request() const {
  // @@protoc_insertion_point(field_get:frpc.RPCMessage.request)
  return _internal_request();
}
template <typename ArgT0, typename... ArgT>
inline PROTOBUF_ALWAYS_INLINE
void RPCMessage::set_request(ArgT0&& arg0, ArgT... args) {
 _has_bits_[0] |= 0x00000004u;
 request_.SetBytes(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, static_cast<ArgT0 &&>(arg0), args..., GetArenaForAllocation());
  // @@protoc_insertion_point(field_set:frpc.RPCMessage.request)
}
inline std::string* RPCMessage::mutable_request() {
  std::string* _s = _internal_mutable_request();
  // @@protoc_insertion_point(field_mutable:frpc.RPCMessage.request)
  return _s;
}
inline const std::string& RPCMessage::_internal_request() const {
  return request_.Get();
}
inline void RPCMessage::_internal_set_request(const std::string& value) {
  _has_bits_[0] |= 0x00000004u;
  request_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, value, GetArenaForAllocation());
}
inline std::string* RPCMessage::_internal_mutable_request() {
  _has_bits_[0] |= 0x00000004u;
  return request_.Mutable(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, GetArenaForAllocation());
}
inline std::string* RPCMessage::release_request() {
  // @@protoc_insertion_point(field_release:frpc.RPCMessage.request)
  if (!_internal_has_request()) {
    return nullptr;
  }
  _has_bits_[0] &= ~0x00000004u;
  auto* p = request_.ReleaseNonDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArenaForAllocation());
#ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
  if (request_.IsDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited())) {
    request_.Set(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), "", GetArenaForAllocation());
  }
#endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
  return p;
}
inline void RPCMessage::set_allocated_request(std::string* request) {
  if (request != nullptr) {
    _has_bits_[0] |= 0x00000004u;
  } else {
    _has_bits_[0] &= ~0x00000004u;
  }
  request_.SetAllocated(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), request,
      GetArenaForAllocation());
#ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
  if (request_.IsDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited())) {
    request_.Set(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), "", GetArenaForAllocation());
  }
#endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
  // @@protoc_insertion_point(field_set_allocated:frpc.RPCMessage.request)
}

// optional bytes response = 6;
inline bool RPCMessage::_internal_has_response() const {
  bool value = (_has_bits_[0] & 0x00000008u) != 0;
  return value;
}
inline bool RPCMessage::has_response() const {
  return _internal_has_response();
}
inline void RPCMessage::clear_response() {
  response_.ClearToEmpty();
  _has_bits_[0] &= ~0x00000008u;
}
inline const std::string& RPCMessage::response() const {
  // @@protoc_insertion_point(field_get:frpc.RPCMessage.response)
  return _internal_response();
}
template <typename ArgT0, typename... ArgT>
inline PROTOBUF_ALWAYS_INLINE
void RPCMessage::set_response(ArgT0&& arg0, ArgT... args) {
 _has_bits_[0] |= 0x00000008u;
 response_.SetBytes(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, static_cast<ArgT0 &&>(arg0), args..., GetArenaForAllocation());
  // @@protoc_insertion_point(field_set:frpc.RPCMessage.response)
}
inline std::string* RPCMessage::mutable_response() {
  std::string* _s = _internal_mutable_response();
  // @@protoc_insertion_point(field_mutable:frpc.RPCMessage.response)
  return _s;
}
inline const std::string& RPCMessage::_internal_response() const {
  return response_.Get();
}
inline void RPCMessage::_internal_set_response(const std::string& value) {
  _has_bits_[0] |= 0x00000008u;
  response_.Set(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, value, GetArenaForAllocation());
}
inline std::string* RPCMessage::_internal_mutable_response() {
  _has_bits_[0] |= 0x00000008u;
  return response_.Mutable(::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::EmptyDefault{}, GetArenaForAllocation());
}
inline std::string* RPCMessage::release_response() {
  // @@protoc_insertion_point(field_release:frpc.RPCMessage.response)
  if (!_internal_has_response()) {
    return nullptr;
  }
  _has_bits_[0] &= ~0x00000008u;
  auto* p = response_.ReleaseNonDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArenaForAllocation());
#ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
  if (response_.IsDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited())) {
    response_.Set(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), "", GetArenaForAllocation());
  }
#endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
  return p;
}
inline void RPCMessage::set_allocated_response(std::string* response) {
  if (response != nullptr) {
    _has_bits_[0] |= 0x00000008u;
  } else {
    _has_bits_[0] &= ~0x00000008u;
  }
  response_.SetAllocated(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), response,
      GetArenaForAllocation());
#ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
  if (response_.IsDefault(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited())) {
    response_.Set(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), "", GetArenaForAllocation());
  }
#endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
  // @@protoc_insertion_point(field_set_allocated:frpc.RPCMessage.response)
}

// optional .frpc.ErrorCode error_code = 7;
inline bool RPCMessage::_internal_has_error_code() const {
  bool value = (_has_bits_[0] & 0x00000040u) != 0;
  return value;
}
inline bool RPCMessage::has_error_code() const {
  return _internal_has_error_code();
}
inline void RPCMessage::clear_error_code() {
  error_code_ = 0;
  _has_bits_[0] &= ~0x00000040u;
}
inline ::frpc::ErrorCode RPCMessage::_internal_error_code() const {
  return static_cast< ::frpc::ErrorCode >(error_code_);
}
inline ::frpc::ErrorCode RPCMessage::error_code() const {
  // @@protoc_insertion_point(field_get:frpc.RPCMessage.error_code)
  return _internal_error_code();
}
inline void RPCMessage::_internal_set_error_code(::frpc::ErrorCode value) {
  _has_bits_[0] |= 0x00000040u;
  error_code_ = value;
}
inline void RPCMessage::set_error_code(::frpc::ErrorCode value) {
  _internal_set_error_code(value);
  // @@protoc_insertion_point(field_set:frpc.RPCMessage.error_code)
}

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__

// @@protoc_insertion_point(namespace_scope)

}  // namespace frpc

PROTOBUF_NAMESPACE_OPEN

template <> struct is_proto_enum< ::frpc::MessageType> : ::std::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor< ::frpc::MessageType>() {
  return ::frpc::MessageType_descriptor();
}
template <> struct is_proto_enum< ::frpc::ErrorCode> : ::std::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor< ::frpc::ErrorCode>() {
  return ::frpc::ErrorCode_descriptor();
}

PROTOBUF_NAMESPACE_CLOSE

// @@protoc_insertion_point(global_scope)

#include <google/protobuf/port_undef.inc>
#endif  // GOOGLE_PROTOBUF_INCLUDED_GOOGLE_PROTOBUF_INCLUDED_rpc_2eproto
