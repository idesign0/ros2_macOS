#include "apriltag_mit/AprilTags/Line2D.h"

namespace AprilTags {

Line2D::Line2D(float k, float b) : dx_(1), dy_(k), p_(0, b) {}

Line2D::Line2D(float dx, float dy, const cv::Point2f &p)
    : dx_(dx), dy_(dy), p_(p) {}

Line2D::Line2D(const cv::Point2f &p0, const cv::Point2f &p1)
    : dx_(p1.x - p0.x), dy_(p1.y - p0.y), p_(p0) {}

Line2D::Line2D(const Segment &seg) : Line2D(seg.p0(), seg.p1()) {}

float Line2D::GetLineCoordinate(const cv::Point2f &p) {
  NormalizeSlope();
  return p.x * dx_ + p.y * dy_;
}

cv::Point2f Line2D::GetPointOfCoordinate(float coord) {
  NormalizeP();
  return cv::Point2f(p_.x + coord * dx_, p_.y + coord * dy_);
}

cv::Point2f Line2D::IntersectionWidth(const Line2D &line) const {
  float m00 = dx_;
  float m01 = -line.dx();
  float m10 = dy_;
  float m11 = -line.dy();

  // determinant of 'm'
  float det = m00 * m11 - m01 * m10;

  // parallel lines? if so, return (-1,0).
  if (fabs(det) < 1e-10)
    return cv::Point2f(-1, 0);

  // inverse of 'm'
  float i00 = m11 / det;
  // float i11 = m00/det;
  float i01 = -m01 / det;
  // float i10 = -m10/det;

  float b00 = line.x() - p_.x;
  float b10 = line.y() - p_.y;

  float x00 = i00 * b00 + i01 * b10;

  return cv::Point2f(dx_ * x00 + p_.x, dy_ * x00 + p_.y);
}

Line2D Line2D::LsqFitXyw(const std::vector<cv::Point3f> &xyw) {
  float Cxx = 0, Cyy = 0, Cxy = 0, Ex = 0, Ey = 0, mXX = 0, mYY = 0, mXY = 0,
        mX = 0, mY = 0;
  float n = 0;

  int idx = 0;
  for (size_t i = 0; i < xyw.size(); i++) {
    const float x = xyw[i].x;
    const float y = xyw[i].y;
    const float w = xyw[i].z;

    mY += y * w;
    mX += x * w;
    mYY += y * y * w;
    mXX += x * x * w;
    mXY += x * y * w;
    n += w;

    idx++;
  }

  Ex = mX / n;
  Ey = mY / n;
  Cxx = mXX / n - Square(mX / n);
  Cyy = mYY / n - Square(mY / n);
  Cxy = mXY / n - (mX / n) * (mY / n);

  // find dominant direction via SVD
  float phi = 0.5f * std::atan2(-2 * Cxy, (Cyy - Cxx));
  // float rho = Ex*cos(phi) + Ey*sin(phi); //why is this needed if he never
  // uses it?
  const auto pts = cv::Point2f(Ex, Ey);

  // compute line parameters
  return Line2D(-std::sin(phi), std::cos(phi), pts);
}

void Line2D::NormalizeSlope() {
  if (!slope_normalized_) {
    float mag = std::sqrt(dx_ * dx_ + dy_ * dy_);
    dx_ /= mag;
    dy_ /= mag;
    slope_normalized_ = true;
  }
}

void Line2D::NormalizeP() {
  if (!p_normalized_) {
    NormalizeSlope();
    // we already have a point (P) on the line, and we know the line vector U
    // and its perpendicular vector V: so, P'=P.*V *V
    float dotprod = -dy_ * p_.x + dx_ * p_.y;
    p_.x = -dy_ * dotprod;
    p_.y = dx_ * dotprod;
    p_normalized_ = true;
  }
}

} // namespace
