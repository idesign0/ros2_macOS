#ifndef APRILTAGS_LINE2D_H_
#define APRILTAGS_LINE2D_H_

#include <cmath>
#include <utility>
#include <vector>
#include <opencv2/core/core.hpp>

#include "apriltag_mit/AprilTags/MathUtil.h"
#include "apriltag_mit/AprilTags/Segment.h"

namespace AprilTags {

//! A 2D line
class Line2D {
 public:
  Line2D() = default;

  Line2D(float k, float b);

  Line2D(const Segment &seg);

  Line2D(float dx, float dy, const cv::Point2f &p);

  Line2D(const cv::Point2f &p1, const cv::Point2f &p2);

  /**
   * @brief getLineCoordinate Get the coordinate of a point on this line.
   * With 0 corresponding to the point on a line that is perpendicular to the
   * line and passing through the origin
   * @param p
   * @return
   */
  float GetLineCoordinate(const cv::Point2f &p);

  //! The inverse of getLineCoordinate.
  cv::Point2f GetPointOfCoordinate(float coord);

  //!Compute the point where two lines intersect, or (-1,0) if the lines are
  // parallel.
  cv::Point2f IntersectionWidth(const Line2D &line) const;

  static Line2D LsqFitXyw(const std::vector<cv::Point3f> &xyw);

  float dx() const { return dx_; }
  float dy() const { return dy_; }
  float x() const { return p_.x; }
  float y() const { return p_.y; }
  const cv::Point2f &p() const { return p_; }

 protected:
  void NormalizeSlope();
  void NormalizeP();

 private:
  float dx_ = 0;
  float dy_ = 0;

  /**
   * @brief p A point the line passes through
   * When normalized, it is the point closest to the origin
   */
  cv::Point2f p_;
  bool slope_normalized_ = false;
  bool p_normalized_ = false;
};

}  // namespace AprilTags

#endif  // APRILTAGS_GLINE2D_H_
