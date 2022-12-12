#ifndef SRC_DB_SNAPSHOT_HPP
#define SRC_DB_SNAPSHOT_HPP

#include <cstdint>
#include <vector>

namespace fver {

namespace db {
class SnapShot {
 public:
  ~SnapShot() = default;
  SnapShot() = default;

  /**
   * @brief: 对 snapshot 进行初始化.
   * @number: 传入的 sequence_number
   * @memtable_n: 当前数据库的 memtable 的数量
   */
  void Init(const uint64_t number, const uint32_t memtable_n);

  auto* getSnapShotData() { return &snapshot_data_; }

  auto* getPeerData(const uint32_t number) {
    return &snapshot_data_[number];
  }

 private:
  uint64_t sequence_number_;
  std::vector<std::vector<char>> snapshot_data_;
};

}  // namespace db
}  // namespace fver

#endif