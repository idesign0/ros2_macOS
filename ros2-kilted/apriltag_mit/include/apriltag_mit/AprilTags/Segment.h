#ifndef APRILTAGS_SEGMENT_H_
#define APRILTAGS_SEGMENT_H_

#include <vector>
#include <opencv2/core/core.hpp>

namespace AprilTags {

//! Represents a line fit to a set of pixels whose gradients are similiar.
class Segment {
 public:
  Segment();

  /**
   * @brief minimumSegmentSize
   * Minimum number of pixels in a segment before we'll fit a line to it
   */
  static constexpr int kMinSegmentPixels = 40;

  /**
   * @brief minimumLineLength
   * Calculated based on minimum plausible decoding size for Tag9 family
   */
  static constexpr float kMinLineLength = 24;

  const cv::Point2f& p0() const { return p0_; }
  void set_p0(const cv::Point2f& p0) { p0_ = p0; }
  const cv::Point2f& p1() const { return p1_; }
  void set_p1(const cv::Point2f& p1) { p1_ = p1; }

  float x0() const { return p0_.x; }
  void set_x0(float x0) { p0_.x = x0; }

  float y0() const { return p0_.y; }
  void set_y0(float y0) { p0_.y = y0; }

  float x1() const { return p1_.x; }
  void set_x1(float x1) { p1_.x = x1; }

  float y1() const { return p1_.y; }
  void set_y1(float y1) { p1_.y = y1; }

  float theta() const { return theta_; }
  void set_theta(float theta) { theta_ = theta; }

  float length() const { return length_; }
  void set_length(float length) { length_ = length; }

  int id() const { return id_; }

  std::vector<Segment*> children;

 private:
  cv::Point2f p0_, p1_;
  float theta_;   // gradient direction (points towards white)
  float length_;  // length of line segment in pixels
  int id_;
  static int counter;
};

}  // namsepace AprilTags

#endif  // APRILTAGS_SEGMENT_H_
