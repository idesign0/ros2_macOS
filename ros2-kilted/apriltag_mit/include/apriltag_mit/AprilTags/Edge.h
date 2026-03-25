#ifndef APRILTAGS_EDGE_H_
#define APRILTAGS_EDGE_H_

#include <vector>
#include <unordered_set>

#include "apriltag_mit/AprilTags/FloatImage.h"
#include "apriltag_mit/AprilTags/DisjointSets.h"

namespace AprilTags {

//! Represents an edge between adjacent pixels in the image.
/*! The edge is encoded by the indices of the two pixels. Edge cost
 *  is proportional to the difference in local orientations.
 */
struct Edge {
  // minimum intensity gradient for an edge to be recognized
  // float const Edge::minMag = 0.004f;
  static constexpr float kMinMag = 0.06f;

  // maximum acceptable difference in local orientation
  static constexpr float kMaxThetaDiff = 25.f * M_PI / 180.f;
  // used to convert cost to int
  static constexpr int kWeightScale = 100;
  // theta threshold for merging edges
  static constexpr float kThetaThresh = 100;
  // magnitude threshold for merging edges
  //  static constexpr float kMagThresh = 1200;
  static constexpr float kMagThresh = 150;

  int pid0;
  int pid1;
  int cost = -1;

  Edge() = default;
  Edge(int pid0, int pid1, int cost) : pid0(pid0), pid1(pid1), cost(cost) {}

  //! Compare edges based on cost
  inline bool operator<(const Edge &other) const { return cost < other.cost; }

  /**
   * @brief EdgeCost Cost of an edge between two adjacent pixels; -1 no edge
   * An edge exists between adjacent pixels if the magnitude of the intensity
   * gradient at both pixels is above threshold. The edge cost is propportional
   * to
   * the difference in the local orientation at the two pixels. Lower cost is
   * better. A cost of -1 means there is no edge here
   * @param theta0
   * @param theta1
   * @param mag1
   * @return
   */
  static int EdgeCost(float theta0, float theta1, float mag1);

  //! Process edges in order of increasing cost, merging clusters if we can do
  // so without exceeding the thetaThresh.
};

struct Stats {
  float mmin, mmax, tmin, tmax;
};

std::vector<Edge> CalcLocalEdges(int x, int y, const FloatImage &im_mag,
                                 const FloatImage &im_theta);

void MergeEdges(const std::vector<Edge> &edges, DisjointSets &dsets,
                std::vector<Stats> &stats, float mag_thresh,
                float theta_thresh);

}  // namespace AprilTags

#endif  // APRILTAGS_EDGE_H_
