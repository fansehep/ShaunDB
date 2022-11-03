#ifndef SRC_UTIL_CRC32_H_
#define SRC_UTIL_CRC32_H_

#include <cstddef>
#include <cstdint>

namespace fver {

namespace util {

// Return the crc32c of concat(A, data[0,n-1]) where init_crc is the
// crc32c of some string A.  Extend() is often used to maintain the
// crc32c of a stream of data.
//
// 返回concat(A, data[0,n-1])的crc32c，其中init_crc是某个字符串A的crc32c。
// 某个字符串A的crc32c。
// 数据流的crc32c。
uint32_t Extend(uint32_t init_crc, const char* data, size_t n);

// Return the crc32c of data[0,n-1]
inline uint32_t Value(const char* data, size_t n) { return Extend(0, data, n); }

static const uint32_t kMaskDelta = 0xa282ead8ul;

// Return a masked representation of crc.
//
// Motivation: it is problematic to compute the CRC of a string that
// contains embedded CRCs.  Therefore we recommend that CRCs stored
// somewhere (e.g., in files) should be masked before being stored.
//
// 返回crc的屏蔽表示法。
//
// 动机：计算一个包含内嵌CRC的字符串的CRC是有问题的。
// 包含嵌入式 CRC。 因此，我们建议存储在某处的CRC
// 因此，我们建议在某个地方（如文件中）存储的CRC应该在存储前被屏蔽。
inline uint32_t Mask(uint32_t crc) {
  // Rotate right by 15 bits and add a constant.
  return ((crc >> 15) | (crc << 17)) + kMaskDelta;
}

// Return the crc whose masked representation is masked_crc.
// 返回被掩盖的crc，其掩盖的表示法为masked_crc。
inline uint32_t Unmask(uint32_t masked_crc) {
  uint32_t rot = masked_crc - kMaskDelta;
  return ((rot >> 17) | (rot << 15));
}

}  // namespace util

}  // namespace fver

#endif