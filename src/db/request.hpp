#ifndef SRC_DB_REQUEST_H_
#define SRC_DB_REQUEST_H_

#include <cstring>
#include <functional>
#include <memory>
#include <string>

#include "src/db/status.hpp"

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

struct GetContextBatch {

};


struct DeleteContextBatch {

};


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

struct DeleteContext {
  DeleteContext() = default;
  std::string key;
  Status code;
  uint64_t number;
};

struct ExistContext {
  ExistContext() = default;
  std::string key;
  bool is_exist;
  Status code;
};

// MemTaskWorker 将负责将 read_only_memtable 删除
struct RemoveReadOnlyMemTableContext {
  RemoveReadOnlyMemTableContext() = default;
};


}  // namespace db

}  // namespace fver

#endif