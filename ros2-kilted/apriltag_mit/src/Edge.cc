#include "apriltag_mit/AprilTags/Edge.h"
#include "apriltag_mit/AprilTags/DisjointSets.h"
#include "apriltag_mit/AprilTags/FloatImage.h"
#include "apriltag_mit/AprilTags/MathUtil.h"

namespace AprilTags {

int Edge::EdgeCost(float theta0, float theta1, float mag1) {
  // mag0 was checked by the main routine so no need to recheck here
  if (mag1 < kMinMag)
    return -1;

  const float theta_diff = std::abs(Mod2Pi(theta1 - theta0));
  if (theta_diff > kMaxThetaDiff)
    return -1;

  const float norm_diff = theta_diff / kMaxThetaDiff;
  return norm_diff * kWeightScale;
}

std::vector<Edge> CalcLocalEdges(int x, int y, const FloatImage &im_mag,
                                 const FloatImage &im_theta) {
  std::vector<Edge> edges;
  edges.reserve(4);

  const int width = im_theta.width();
  const int pid0 = y * width + x;
  const float theta0 = im_theta.get(pid0);

  const int pid1s[4] = {pid0 + 1, pid0 + width, pid0 + width + 1,
                        pid0 + width - 1};
  int n = 4;
  // Handle a corner case when pixel is on left border
  if (x == 0)
    n = 3;

  for (int i = 0; i < n; ++i) {
    const auto pid1 = pid1s[i];
    const auto cost =
        Edge::EdgeCost(theta0, im_theta.get(pid1), im_mag.get(pid1));
    if (cost >= 0) {
      edges.emplace_back(pid0, pid1, cost);
    }
  }

  return edges;
}

void MergeEdges(const std::vector<Edge> &edges, DisjointSets &dsets,
                std::vector<Stats> &stats, float mag_thresh,
                float theta_thresh) {
  for (const Edge &e : edges) {
    const int id0 = dsets.Find(e.pid0);
    const int id1 = dsets.Find(e.pid1);

    if (id0 == id1)
      continue;

    const int sz0 = dsets.GetSetSize(id0);
    const int sz1 = dsets.GetSetSize(id1);
    const int sz01 = sz0 + sz1;

    const Stats &s0 = stats[id0];
    const Stats &s1 = stats[id1];

    const float mcost0 = s0.mmax - s0.mmin;
    const float mcost1 = s1.mmax - s1.mmin;
    const float tcost0 = s0.tmax - s0.tmin;
    const float tcost1 = s1.tmax - s1.tmin;
    const float tmean0 = (s0.tmin + s0.tmax) / 2;
    const float tmean1 = (s1.tmin + s1.tmax) / 2;

    // bshift will be a multiple of 2pi that aligns the spans of 'b' with 'a'
    // so that we can properly take the union of them.
    const float bshift = Mod2Pi(tmean0, tmean1) - tmean1;

    const float mmin01 = std::min(s0.mmin, s1.mmin);
    const float mmax01 = std::max(s0.mmax, s1.mmax);
    float tmin01 = std::min(s0.tmin, s1.tmin + bshift);
    float tmax01 = std::max(s0.tmax, s1.tmax + bshift);

    // Corner case probably not too useful to hand correctly
    if (tmax01 - tmin01 > 2 * Pi<float>()) {
      tmax01 = tmin01 + 2 * Pi<float>();
    }

    // merge these two clusters?
    const float mcost01 = mmax01 - mmin01;
    const float tcost01 = tmax01 - tmin01;

    if (tcost01 <= (std::min(tcost0, tcost1) + theta_thresh / sz01) &&
        mcost01 <= std::min(mcost0, mcost1) + mag_thresh / sz01) {
      const int id01 = dsets.Union(id0, id1);

      Stats &s01 = stats[id01];
      s01.tmax = tmax01;
      s01.tmin = tmin01;
      s01.mmax = mmax01;
      s01.mmin = mmin01;
    }
  }
}

} // namespace AprilTags
