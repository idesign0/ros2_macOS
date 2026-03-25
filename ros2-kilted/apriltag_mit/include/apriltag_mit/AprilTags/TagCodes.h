#ifndef APRILTAGS_TAGCODES_H_
#define APRILTAGS_TAGCODES_H_

#include <vector>
#include <cmath>

using code_t = unsigned long long;

namespace AprilTags {
struct TagCodes {
  TagCodes(unsigned payload_bits, unsigned min_hamming,
           const std::vector<code_t>& codes)
      : payload_bits(payload_bits),
        dimension_bits(std::sqrt(payload_bits)),
        min_hamming(min_hamming),
        codes(codes) {}

  unsigned payload_bits, dimension_bits, min_hamming;
  std::vector<code_t> codes;
};

}  // namespace AprilTags

#endif  // APRILTAGS_TAGCODES_H
