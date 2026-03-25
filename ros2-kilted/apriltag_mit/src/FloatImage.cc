#include "apriltag_mit/AprilTags/FloatImage.h"
#include <opencv2/imgproc/imgproc.hpp>

namespace AprilTags {

FloatImage::FloatImage(int width, int height) {
  image_.create(height, width, CV_32FC1);
}

FloatImage::FloatImage(const cv::Mat &image) {
  image.convertTo(image_, CV_32FC1, 1.0 / 255);
}

FloatImage &FloatImage::operator=(const FloatImage &other) {
  other.image_.copyTo(image_);
  return *this;
}

void FloatImage::FilterGaussian(int ksize, float sigma) {
  cv::GaussianBlur(image_, image_, cv::Size(ksize, ksize), sigma);
}

bool IsInsideImage(int x, int y, int w, int h) {
  return (x >= 0 && x < w && y >= 0 && y < h);
}

bool IsInsideImage(int x, int y, const FloatImage &image) {
  return IsInsideImage(x, y, image.width(), image.height());
}

} // namespace AprilTags
