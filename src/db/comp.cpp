#include "src/db/comp.hpp"

#include "src/base/log/logging.hpp"
#include "src/db/key_format.hpp"
namespace fver {

namespace db {

bool Comparator::operator()(std::string_view const& lef,
                            std::string_view const& ref) const {
  // uint32_t lef_key_size = formatDecodeFixed32(lef.data());
  // uint32_t ref_key_size = formatDecodeFixed32(ref.data());
  // std::string_view key_view(lef.data() + 4, lef_key_size);
  // std::string_view value_view(ref.data() + 4, ref_key_size);
  // return key_view > value_view;
  //   uint32_t lef_key_size =
  //       static_cast<uint32_t>(lef[0]) | static_cast<uint32_t>(lef[1]) << 8 |
  //       static_cast<uint32_t>(lef[2]) << 16 | static_cast<uint32_t>(lef[3])
  //       << 24;
  //   uint32_t ref_key_size =
  //       static_cast<uint32_t>(ref[0]) | static_cast<uint32_t>(ref[1]) << 8 |
  //       static_cast<uint32_t>(ref[2]) << 16 | static_cast<uint32_t>(ref[3])
  //       << 24;
  //   std::string_view lef_key_view(lef.data() + 4, lef_key_size);
  //   std::string_view ref_key_view(ref.data() + 4, ref_key_size);
  uint32_t lef_key_size = 0;
  auto end_ptr = getVarint32Ptr(lef.data(), lef.data() + 5, &lef_key_size);
  assert(end_ptr != nullptr);
  uint32_t ref_key_size = 0;
  auto end_2_ptr = getVarint32Ptr(ref.data(), ref.data() + 5, &ref_key_size);
  assert(end_ptr != nullptr);
  std::string_view lef_key_view(end_ptr, lef_key_size);
  std::string_view ref_key_view(end_2_ptr, ref_key_size);
  return lef_key_view < ref_key_view;
}

}  // namespace db

}  // namespace fver