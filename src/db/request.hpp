#ifndef SRC_DB_REQUEST_H_
#define SRC_DB_REQUEST_H_

#include <cstring>
#include <functional>
#include <memory>
#include <string>

#include "src/db/status.hpp"
#include "src/db/snapshot.hpp"

namespace fver {

namespace db {

enum Request : uint32_t {
  kSet_Request,
  kPut_Request,
  kGet_Request,
  kDel_Request,
  KExi_Request,
};

// 这里采取string 复制的方式, 有些许性能开销
struct SetContext {
  SetContext() = default;
  std::string key;
  std::string value;
  Status code;
  uint64_t number;
  SetContext(const std::string_view& key_view,
             const std::string_view& value_view)
      : key(key_view), value(value_view) {}
};

// TODO: 增加 batch 操作
struct SetContextBatch {
  SetContextBatch() = default;
  std::vector<std::pair<std::string, std::string>> kv_vec;
  Status code;
  uint64_t number;
  void AddKVRecord(const std::pair<std::string, std::string>& add_record) {
    kv_vec.push_back(add_record);
  }
};

struct PutContext {
  PutContext() = default;
  std::string key;
  std::string value;
  Status code;
  uint64_t number;
};

struct PutContextBatch {
  PutContextBatch() = default;
  std::vector<std::pair<std::string, std::string>> kv_vec;
  Status code;
  uint64_t number;
  void AddKVRecord(const std::pair<std::string, std::string>& add_record) {
    kv_vec.push_back(add_record);
  }
};

struct GetContextBatch {};

struct DeleteContextBatch {};

// TODO: get request need a condition_variable to notify the wait thread
//  to tell the
struct GetContext {
  GetContext() = default;
  std::string key;
  std::string value;
  Status code;
  uint64_t number;
  // 由于是异步, 所以要在这里做回调
  std::function<void(const std::shared_ptr<GetContext>&)> get_callback;

  GetContext(const std::string_view& key_view)
      : key(key_view), get_callback(nullptr) {}
};

struct GetContext_view {
  std::string_view key;
  std::string_view value;
  Status code;
};

struct DeleteContext {
  DeleteContext() = default;
  std::string key;
  Status code;
  uint64_t number;
  //
  std::function<void(const std::shared_ptr<DeleteContext>&)> del_callback;
};

struct ExistContext {
  ExistContext() = default;
  std::string key;
  bool is_exist;
  Status code;
};


struct DeleteRecordContext {
  std::string_view key;
};

struct SnapShotContext {
  SnapShot snapshot;
  Status code;
};

}  // namespace db

}  // namespace fver

#endif