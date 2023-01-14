#ifndef SRC_UTIL_BITSET_H_
#define SRC_UTIL_BITSET_H_

#include <bitset>
#include <cstring>
#include <new>
#include <string>

#include "src/base/log/logging.hpp"

extern "C" {
#include <assert.h>
}

namespace fver {

namespace util {

constexpr uint8_t kset_Remain_Idx[] = {
    1,    // binary: 0000 0001
    2,    //         0000 0010
    4,    //         0000 0100
    8,    //         0000 1000
    16,   //         0001 0000
    32,   //         0010 0000
    64,   //         0100 0000
    128,  //         1000 0000
};

constexpr uint8_t kdel_Remain_Idx[] = {
    254,  // binary: 1111 1110
    253,  //         1111 1101
    251,  //         1111 1011
    247,  //         1111 0111
    239,  //         1110 1111
    223,  //         1101 1111
    191,  //         1011 1111
    127,  //         0111 1111
};

// 2^5 = 32
constexpr uint32_t kshiftSize = 5;
constexpr uint32_t kmask = 0x1f;

template <int N>
class BitSet {
  static_assert(N >= 64, "bitset size must >= 64");

 public:
  // 析构函数
  ~BitSet() { delete[] data; }

  constexpr BitSet() : data(nullptr), data_size(N / 8) {
    // 一个字节占 8 位
    data = new (std::nothrow) char[(N / 8)];
    assert(data != nullptr);
    std::memset(data, 0, data_size);
  }

  // 作为视图,
  BitSet(char* data_view, const uint32_t data_size)
      : data(data_view), data_size(data_size) {}

  // 将某一个位设置为 1
  void set(uint32_t idx) {
    auto pre_idx = idx / 8;
    auto remain_idx = idx % 8;
    data[pre_idx] |= kset_Remain_Idx[remain_idx];
  }

  // 将某个位设置为 0
  void del(uint32_t idx) {
    auto pre_idx = idx / 8;
    auto remain_idx = idx % 8;
    data[pre_idx] &= kdel_Remain_Idx[remain_idx];
  }

  // 判断某个位是否是 true
  bool test(uint32_t idx) {
    auto pre_idx = idx / 8;
    auto remain_idx = idx % 8;
    return data[pre_idx] & kset_Remain_Idx[remain_idx];
  }

  // 获取 data_size
  uint32_t getSize() { return data_size; }

  // 获取 data
  auto getData() { return data; }

 private:
  char* data;
  const uint32_t data_size;
};

}  // namespace util
}  // namespace fver

#endif