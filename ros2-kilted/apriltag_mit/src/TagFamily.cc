#include <iostream>
#include <numeric>

#include "apriltag_mit/AprilTags/MathUtil.h"
#include "apriltag_mit/AprilTags/TagFamily.h"

namespace AprilTags {

using namespace std;

TagFamily::TagFamily(const TagCodes &tag_codes)
    : tag_codes_(tag_codes), num_codes_(tag_codes.codes.size()),
      error_recovery_bits_(1) {}

unsigned TagFamily::payload_bits() const { return tag_codes_.payload_bits; }
unsigned TagFamily::dimension_bits() const { return tag_codes_.dimension_bits; }
unsigned TagFamily::min_hamming() const { return tag_codes_.min_hamming; }
const std::vector<code_t> &TagFamily::codes() const { return tag_codes_.codes; }
size_t TagFamily::num_codes() const { return num_codes_; }

void TagFamily::set_error_recovery_bits(unsigned error_recovery_bits) {
  error_recovery_bits_ = error_recovery_bits;
}
void TagFamily::set_error_recovery_fraction(float v) {
  error_recovery_bits_ = static_cast<unsigned>(((min_hamming() - 1) / 2) * v);
}

code_t TagFamily::Code(unsigned id) const { return codes()[id]; }

bool TagFamily::IsGood(unsigned id, unsigned hamming_distance) const {
  return (id != std::numeric_limits<unsigned>::max()) &&
         (hamming_distance <= error_recovery_bits_);
}

TagDetection TagFamily::Decode(code_t obs_code) const {
  auto best_id = std::numeric_limits<unsigned>::max();
  auto best_hamming = std::numeric_limits<unsigned>::max();
  unsigned best_rotation = 0;

  code_t rot_codes[4];
  rot_codes[0] = obs_code;
  rot_codes[1] = Rotate90DegCwise(rot_codes[0], dimension_bits());
  rot_codes[2] = Rotate90DegCwise(rot_codes[1], dimension_bits());
  rot_codes[3] = Rotate90DegCwise(rot_codes[2], dimension_bits());

  for (size_t id = 0; id < num_codes_; ++id) {
    for (size_t rot = 0; rot < 4; ++rot) {
      const auto hamming = HammingDistance(rot_codes[rot], Code(id));
      if (hamming == 0) {
        // Hamming distance is 0, this is our tag, no need to test against other
        // tags
        return TagDetection(id, true, obs_code, Code(id), hamming, rot);
      } else if (hamming < best_hamming) {
        best_id = id;
        best_hamming = hamming;
        best_rotation = rot;
      }
    }
  }

  return TagDetection(best_id, IsGood(best_id, best_hamming), obs_code,
                      Code(best_id), best_hamming, best_rotation);
}

TagDetection TagFamily::DecodeQuad(const Quad &quad, const FloatImage &image,
                                   unsigned black_border) const {
  // Get code from this quad if possible
  // NOTE: In the original paper, this step will require the calculation of a
  // homography. But in Kaess' code, this is replaced by a simple bilinear
  // interpolation. This is not as accurate as the homography, but it works most
  // of the times
  const auto code = quad.ToTagCode(image, dimension_bits(), black_border);

  // We get a bad code, just return a bad tag detection
  if (code == 0)
    return TagDetection();

  auto td = Decode(code);
  td.RotatePoints(quad.p);

  if (td.good) {
    td.cxy = quad.Interpolate01({0.5f, 0.5f});
    td.obs_perimeter = quad.obs_perimeter;
  }

  return td;
}

code_t Rotate90DegCwise(code_t w, int d) {
  code_t wr = 0;
  const code_t one = 1;

  for (int r = d - 1; r >= 0; r--) {
    for (int c = 0; c < d; c++) {
      int b = r + d * c;
      wr = wr << 1;

      if ((w & (one << b)) != 0)
        wr |= 1;
    }
  }
  return wr;
}

unsigned HammingDistance(code_t a, code_t b) {
  // Because code_t is unsigned long long
  return __builtin_popcountll(a ^ b);
}

} // namespace AprilTags
