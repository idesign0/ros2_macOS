#ifndef APRILTAGS_FLOATIMAGE_H_
#define APRILTAGS_FLOATIMAGE_H_

#include <opencv2/core/core.hpp>

namespace AprilTags {

//! Represent an image as a vector of floats in [0,1]
class FloatImage {
 public:
  FloatImage() = default;
  FloatImage(const cv::Mat& image);
  FloatImage(int width, int height);
  FloatImage& operator=(const FloatImage& other);

  inline float get(int x, int y) const { return image_.at<float>(y, x); }
  inline float get(int idx) const { return image_.at<float>(idx); }
  inline void set(int x, int y, float v) { image_.at<float>(y, x) = v; }

  int width() const { return image_.cols; }
  int height() const { return image_.rows; }
  int num_pixels() const { return image_.cols * image_.rows; }
  cv::Mat& mat() { return image_; }
  const cv::Mat& mat() const { return image_; }

  void FilterGaussian(int ksize, float sigma);

 private:
  cv::Mat image_;
};

bool IsInsideImage(int x, int y, int w, int h);
bool IsInsideImage(int x, int y, const FloatImage& image);

}  // namespace AprilTags

#endif  // APRILTAGS_FLOATIMAGE_H_
