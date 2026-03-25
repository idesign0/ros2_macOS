#ifndef APRILTAGS_QUAD_H_
#define APRILTAGS_QUAD_H_

#include <opencv2/core/core.hpp>
#include "apriltag_mit/AprilTags/GrayModel.h"
#include "apriltag_mit/AprilTags/FloatImage.h"
#include "apriltag_mit/AprilTags/TagCodes.h"

namespace AprilTags {

class FloatImage;
class Segment;

//! Represents four segments that form a loop, and might be a tag.
class Quad {
 public:
  /**
   * @brief kMinEdgeLength Minimum size of a tag (in pixels) as measured along
   * edges and diagonals
   */
  static const int kMinEdgeLength = 6;
  /**
   * @brief kMaxQuadAspectRatio Early pruning of quads with insane ratios
   */
  static constexpr float kMaxQuadAspectRatio = 32.0;

  Quad(const std::vector<cv::Point2f>& p);

  /**
   * @brief interpolate Interpolate given that the lower left corner of the
   * lower left cell is (-1, -1) and the upper right corner of the upper right
   * cell is at (1,1)
   * @param p
   * @return
   */
  cv::Point2f Interpolate(const cv::Point2f& p) const;

  /**
   * @brief interpolate01 Interpolate between 0~1 instead of -1~1
   * @param p
   * @return
   */
  cv::Point2f Interpolate01(const cv::Point2f& p) const;

  /**
   * @brief p Points for the quad in pixel coordinates, in couter clockwise
   * order. These points are the intersectiosn of segments
   */
  std::vector<cv::Point2f> p;

  /**
   * @brief segments Segments composing this quad
   */
  std::vector<Segment*> segments;

  /**
   * @brief obs_perimeter Total length in pixels of the acutal perimeter
   * observed for the quad.
   * This is in contrast to the geometric perimeter, some of which may not have
   * been directly observed but rather inferred by tintersecting segments. Quads
   * with more observed perimeter are preferred over others.
   */
  float obs_perimeter;

  /**
   * @brief search Searches through a vector of segments to form Quads
   * @param path
   * @param parent
   * @param depth
   * @param quads
   */
  static void Search(std::vector<Segment*>& path, Segment& parent, int depth,
                     std::vector<Quad>& quads);

  code_t ToTagCode(const FloatImage& image, unsigned dimension_bits,
                   unsigned black_border) const;

 private:
  GrayModel MakeGrayModel(const FloatImage& image, unsigned length_bits) const;
  code_t DecodePayload(const FloatImage& image, const GrayModel& model,
                       unsigned dimension_bits, unsigned black_border) const;

  cv::Point2f p0_, p3_, p01_, p32_;
};

}  // namespace AprilTags

#endif  // APRILTAGS_QUAD_H_
