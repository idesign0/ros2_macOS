#include "apriltag_mit/AprilTags/LineSegment2D.h"
#include <limits>

namespace AprilTags {

LineSegment2D::LineSegment2D(const cv::Point2f &p0, const cv::Point2f &p1)
    : line_(p0, p1), p0_(p0), p1_(p1) {}

LineSegment2D LineSegment2D::LsqFitXyw(const std::vector<cv::Point3f> &xyws) {
  auto line = Line2D::LsqFitXyw(xyws);
  float coord_max = -std::numeric_limits<float>::infinity();
  float coord_min = std::numeric_limits<float>::infinity();

  for (const cv::Point3f &xyw : xyws) {
    const cv::Point2f p(xyw.x, xyw.y);
    const float coord = line.GetLineCoordinate(p);
    coord_max = std::max(coord_max, coord);
    coord_min = std::min(coord_min, coord);
  }

  const auto p_min = line.GetPointOfCoordinate(coord_min);
  const auto p_max = line.GetPointOfCoordinate(coord_max);
  return LineSegment2D(p_min, p_max);
}

} // namespace AprilTags
