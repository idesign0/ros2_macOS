#ifndef APRILTAGS_GRAYMODEL_H_
#define APRILTAGS_GRAYMODEL_H_

#include <Eigen/Dense>
#include <vector>

namespace AprilTags {

/**
 * @brief The GrayModel class
 * I(x, y) = A*x + B*y + C*xy + D
 */

class IntensityModel {
 public:
  IntensityModel();

  void AddObservation(float x, float y, float v);
  float Predict(float x, float y) const;
  void Fit();

 private:
  // We're solving Av = b.
  //
  // For each observation, we add a row to A of the form [x y xy 1]
  // and to b of the form gray*[x y xy 1].  v is the vector [A B C D].
  //
  // The least-squares solution to the system is v = inv(A'A)A'b

  Eigen::Matrix4d A_;
  Eigen::Vector4d c_;
  Eigen::Vector4d b_;
  int num_obs_;
  bool dirty_;  //!< True if we've added an observation and need to recompute v
};

class GrayModel {
 public:
  GrayModel() = default;

  void AddBlackObs(float x, float y, float v);
  void AddWhiteObs(float x, float y, float v);

  void Fit();
  float CalcThreshold(float x, float y) const;

 private:
  IntensityModel black_model_, white_model_;
};

bool IsOnOuterBorder(int x, int y, int l, bool black_corner = true);
bool IsOnInnerBorder(int x, int y, int l);
bool IsInsideInnerBorder(int x, int y, int l);

}  // namespace AprilTags

#endif  // APRILTAGS_GRAYMODEL_H_
