#include <Eigen/Dense>
#include <Eigen/LU>

#include "apriltag_mit/AprilTags/GrayModel.h"

namespace AprilTags {

void GrayModel::AddBlackObs(float x, float y, float v) {
  black_model_.AddObservation(x, y, v);
}

void GrayModel::AddWhiteObs(float x, float y, float v) {
  white_model_.AddObservation(x, y, v);
}

void GrayModel::Fit() {
  black_model_.Fit();
  white_model_.Fit();
}

float GrayModel::CalcThreshold(float x, float y) const {
  return (black_model_.Predict(x, y) + white_model_.Predict(x, y)) / 2;
}

IntensityModel::IntensityModel()
    : A_(), c_(), b_(), num_obs_(0), dirty_(false) {
  A_.setZero();
  c_.setZero();
  b_.setZero();
}

void IntensityModel::AddObservation(float x, float y, float v) {
  float xy = x * y;

  // update only upper-right elements. A'A is symmetric,
  // we'll fill the other elements in later.
  A_(0, 0) += x * x;
  A_(0, 1) += x * y;
  A_(0, 2) += x * xy;
  A_(0, 3) += x;
  A_(1, 1) += y * y;
  A_(1, 2) += y * xy;
  A_(1, 3) += y;
  A_(2, 2) += xy * xy;
  A_(2, 3) += xy;
  A_(3, 3) += 1;

  b_[0] += x * v;
  b_[1] += y * v;
  b_[2] += xy * v;
  b_[3] += v;

  num_obs_++;
  dirty_ = true;
}

float IntensityModel::Predict(float x, float y) const {
  return c_[0] * x + c_[1] * y + c_[2] * x * y + c_[3];
}

void IntensityModel::Fit() {
  // we really only need 4 linearly independent observations to fit our answer,
  // but we'll be very sensitive to noise if we don't have an over-determined
  // system. Thus, require at least 6 observations (or we'll use a constant
  // model below).

  if (num_obs_ >= 6) {
    // make symmetric
    for (int i = 0; i < 4; ++i) {
      for (int j = i + 1; j < 4; ++j) {
        A_(j, i) = A_(i, j);
      }
    }

    bool invertible;
    double det_unused;
    Eigen::Matrix4d Ainv;
    A_.computeInverseAndDetWithCheck(Ainv, det_unused, invertible);
    if (invertible) {
      c_ = Ainv * b_;
      return;
    }
  }

  // If we get here, either nobs < 6 or the matrix inverse generated
  // an underflow, so use a constant model.
  c_.setZero(); // need the cast to avoid operator= ambiguity wrt. const-ness
  c_[3] = b_[3] / num_obs_;
}

bool IsOnOuterBorder(int x, int y, int l, bool black_corner) {
  if (black_corner) {
    // Black corners will be excluded
    const bool on_corner = (x == -1 && y == -1) || (x == -1 && y == l) ||
                           (x == l && y == -1) || (x == l && y == l);
    return (y == -1 || y == l || x == -1 || x == l) && !on_corner;
  } else {
    return (y == -1 || y == l || x == -1 || x == l);
  }
}

bool IsOnInnerBorder(int x, int y, int l) {
  return (y == 0 || y == (l - 1) || x == 0 || x == (l - 1));
}

bool IsInsideInnerBorder(int x, int y, int l) {
  return (y >= 1 && y < (l - 1) && x >= 1 && x < (l - 1));
}

} // namespace AprilTags
