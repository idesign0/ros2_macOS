#include "apriltag_mit/AprilTags/TagDetection.h"
#include "apriltag_mit/AprilTags/MathUtil.h"
#include "opencv2/opencv.hpp"
#include <iterator>

namespace AprilTags {

float TagPerimeter(const std::vector<cv::Point2f> &p) {
  float perimeter = 0;
  for (size_t i = 0; i < 3; ++i) {
    perimeter += Distance2D(p[i], p[i + 1]);
  }
  return perimeter;
}

float TagRadius(const std::vector<cv::Point2f> &p) {
  return TagPerimeter(p) / 8.0f;
}

cv::Point2f TagDetection::Project(const cv::Point2f &p) const {
  float z = H(2, 0) * p.x + H(2, 1) * p.y + H(2, 2);
  // prevents returning a pair with -NaN, for which gcc 4.4 flubs isnan
  if (z == 0.0)
    return {0, 0};

  const float x = (H(0, 0) * p.x + H(0, 1) * p.y + H(0, 2)) / z;
  const float y = (H(1, 0) * p.x + H(1, 1) * p.y + H(1, 2)) / z;
  return cv::Point2f(x, y);
}

void TagDetection::RotatePoints(const std::vector<cv::Point2f> &quad_p) {
  // Compute the homography (and rotate it appropriately)
  // NOTE: since homography is not needed, we disable it here

  //  H = CalcHomography(quad_p);

  //  float c = std::cos(num_rot * Pi_2<float>());
  //  float s = std::sin(num_rot * Pi_2<float>());
  //  auto R = cv::Matx33f::zeros();
  //  R(0, 0) = R(1, 1) = c;
  //  R(0, 1) = -s;
  //  R(1, 0) = s;
  //  R(2, 2) = 1;
  //  H = H * R;

  // Rotate points in detection according to decoded orientation. Thus the
  // order of the points in the detection object can be used to determine
  // the orientation of the target.
  // NOTE: since we already get the rotation from matching the code, we simply
  // rotate points here, instead of comparing distance as Kaess did in his
  // original code
  for (size_t i = 0; i < 4; ++i) {
    p[i] = quad_p[(i + num_rot) % 4];
  }
}

bool TagDetection::OverlapsTooMuch(const TagDetection &other) const {
  // Compute a sort of "radius" of the two targets. We'll do this by
  // computing the average length of the edges of the quads
  const auto radius = (TagRadius(p) + TagRadius(other.p)) / 2;

  // distance (in pixels) between two tag centers
  const auto dist = Distance2D(cxy, other.cxy);

  // reject pairs where the distance between centroids is smaller than the
  // "radius" of one of the tags.
  return (dist < radius);
}

void TagDetection::ScaleTag(float scale) {
  cxy *= scale;
  for (cv::Point2f &c : p) {
    c *= scale;
  }
}

cv::Matx33f CalcHomography(const std::vector<cv::Point2f> &p) {
  std::vector<cv::Point2f> obj_pts = {{-1, -1}, {1, -1}, {1, 1}, {-1, 1}};
  const auto Hd = cv::findHomography(obj_pts, p);
  cv::Matx33f Hf;
  for (size_t c = 0; c < 3; ++c) {
    for (size_t r = 0; r < 3; ++r) {
      Hf(r, c) = Hd.at<double>(r, c);
    }
  }
  return Hf;
}

} // namespace AprilTags
