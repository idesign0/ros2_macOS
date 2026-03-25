#ifndef APRILTAGS_TAGDETECTION_H_
#define APRILTAGS_TAGDETECTION_H_

#include <vector>
#include <opencv2/core/core.hpp>

#include "apriltag_mit/AprilTags/TagCodes.h"

namespace AprilTags {

using Pointf = std::pair<float, float>;

struct TagDetection {
  TagDetection() = default;
  TagDetection(unsigned id, bool good, code_t obs_code, code_t code,
               unsigned hamming_distance, unsigned num_rotations)
      : id(id),
        good(good),
        obs_code(obs_code),
        code(code),
        hamming_distance(hamming_distance),
        num_rot(num_rotations) {
    p.resize(4);
  }

  unsigned id;
  bool good = false;

  /**
   * @brief obs_code Observed code
   */
  code_t obs_code;

  /**
   * @brief code Matched code
   */
  code_t code;

  /**
   * @brief hamming_distance
   */
  unsigned hamming_distance;

  /**
   * @brief num_rotations Number of 90 degree rotations clockwise required to
   * align the code
   */
  unsigned num_rot;

  /////////////// Fields below are filled in by TagDetector ///////////////

  /**
   * @brief cxy Center of tag in pixel coordinates
   */
  cv::Point2f cxy;

  /**
   * @brief p Position of the detection
   * The points travel counter-clockwise around the target, alwasy starting from
   * the same corner of the tag
   */
  std::vector<cv::Point2f> p;

  /**
   * @brief obs_perimeter length of the observed perimeter
   * Observed perimeter excludes the inferred perimeter which is used to connect
   * incomplete quads
   */
  float obs_perimeter;

  /**
   * @brief H Homography
   * y = Hx, y are pixel coordinates, x are tag-relative coordinates
   * from (-1,-1) to (1, 1)
   */
  cv::Matx33f H;

  /**
   * @brief interpolate
   * @param p
   * @return
   */
  cv::Point2f Project(const cv::Point2f& p) const;

  void RotatePoints(const std::vector<cv::Point2f>& quad_p);

  /**
   * @brief OverlapsTooMuch Determines whether two tags overlap too much
   * @param other
   * @return
   */
  bool OverlapsTooMuch(const TagDetection& other) const;

  /**
   * @brief ScaleTag
   * @param scale
   */
  void ScaleTag(float scale);
};

float TagPerimeter(const std::vector<cv::Point2f>& p);
float TagRadius(const std::vector<cv::Point2f>& p);

cv::Matx33f CalcHomography(const std::vector<cv::Point2f>& p);

}  // namespace AprilTags

#endif  // APRILTAGS_TAGDETECTION_H_
