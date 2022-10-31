#include <gtest/gtest.h>

#include "src/base/log/logging.hpp"
#include "src/raft/test/testkv.pb.h"

namespace fver {

namespace raft {

class ProtoBufTest : public ::testing::Test {
 public:
  ProtoBufTest() = default;
  ~ProtoBufTest() = default;
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(ProtoBufTest, parse_serialze_test) {
  testkv::SetKeyRequest set_key_request;
  set_key_request.set_key("123");
  set_key_request.set_value("456");
  std::string string_set_key;
  set_key_request.SerializeToString(&string_set_key);
  LOG_DEBUG("set_key_request.key: {} value: {} ", set_key_request.key(),
            set_key_request.value());
  LOG_DEBUG("serialize_to_string: {} string_size: {}", string_set_key,
            string_set_key.size());
  testkv::SetKeyRequest new_set_key_request;
  new_set_key_request.ParseFromString(string_set_key);
  ASSERT_EQ(new_set_key_request.key(), "123");
  ASSERT_EQ(new_set_key_request.value(), "456");
}

}  // namespace raft

}  // namespace fver
