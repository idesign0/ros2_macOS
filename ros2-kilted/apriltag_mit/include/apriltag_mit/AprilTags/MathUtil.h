#ifndef APRILTAGS_MATHUTIL_H_
#define APRILTAGS_MATHUTIL_H_

#include <cmath>
#include <cfloat>
#include <cstdlib>
#include <utility>
#include <opencv2/core/core.hpp>

namespace AprilTags {

template <typename T>
constexpr T Pi() {
  static_assert(std::is_floating_point<T>::value,
                "Type must be floating point");
  return static_cast<T>(M_PI);
}

template <typename T>
constexpr T Pi_2() {
  static_assert(std::is_floating_point<T>::value,
                "Type must be floating point");
  return static_cast<T>(M_PI / 2);
}

//! Returns a result in [-Pi, Pi]
static inline float Mod2Pi(float theta) {
  const float twopi = 2 * Pi<float>();
  const float twopi_inv = 1.f / (2.f * Pi<float>());

  const float abs_theta = std::abs(theta);
  const float q = abs_theta * twopi_inv + 0.5f;
  const int qi = (int)q;
  const float r = abs_theta - qi * twopi;
  return (theta < 0) ? -r : r;
}

//! Returns a value of v wrapped such that ref and v differ by no more than
//+/- Pi
static inline float Mod2Pi(float ref, float theta) {
  return ref + Mod2Pi(theta - ref);
}

static inline float Distance2D(const cv::Point2f& p0, const cv::Point2f& p1) {
  const auto dx = p0.x - p1.x;
  const auto dy = p0.y - p1.y;
  return std::sqrt(dx * dx + dy * dy);
}

static inline float Square(float x) { return x * x; }

}  // namespace AprilTags

#endif  // APRILTAGS_MATHUTIL_H_
