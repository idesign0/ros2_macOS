#include <algorithm>
#include <climits>
#include <cmath>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "apriltag_mit/AprilTags/Edge.h"
#include "apriltag_mit/AprilTags/GrayModel.h"
#include "apriltag_mit/AprilTags/Gridder.h"
#include "apriltag_mit/AprilTags/Line2D.h"
#include "apriltag_mit/AprilTags/LineSegment2D.h"
#include "apriltag_mit/AprilTags/MathUtil.h"
#include "apriltag_mit/AprilTags/Segment.h"

#include "apriltag_mit/AprilTags/TagDetector.h"

#include "apriltag_mit/AprilTags/timer.h"

namespace AprilTags {

TagDetector::TagDetector(const TagCodes &tag_codes, int black_border)
    : tag_family_(tag_codes), black_border_(black_border) {}

void TagDetector::set_black_border(int black_border) {
  black_border_ = black_border;
}

int TagDetector::black_border() const { return black_border_; }

int TagDetector::CalcFilterSize(float sigma) const {
  return static_cast<int>(std::max(3.0f, 3 * sigma)) | 1;
}

void TagDetector::Preprocess(const FloatImage &image, FloatImage &im_decode,
                             FloatImage &im_segment) const {
  im_decode = image;
  if (decode_sigma_ > 0) {
    const auto filter_size = CalcFilterSize(decode_sigma_);
    im_decode.FilterGaussian(filter_size, decode_sigma_);
  }

  if (segment_sigma_ > 0) {
    if (segment_sigma_ == decode_sigma_) {
      im_segment.mat() = im_decode.mat();
    } else {
      const auto filter_size = CalcFilterSize(segment_sigma_);
      // Copy
      im_segment = image;
      im_segment.FilterGaussian(filter_size, segment_sigma_);
    }
  } else {
    im_segment.mat() = image.mat();
  }
}

void TagDetector::CalcPolar(const FloatImage &image, FloatImage &im_mag,
                            FloatImage &im_theta) const {
  cv::Mat Ix, Iy;
  // Need to scale the gradient magnitude, because Scharr is 3, 10, 3
  cv::Scharr(image.mat(), Ix, CV_32F, 1, 0, 1 / 16.0);
  cv::Scharr(image.mat(), Iy, CV_32F, 0, 1, 1 / 16.0);
  cv::cartToPolar(Ix, Iy, im_mag.mat(), im_theta.mat());
}

DisjointSets TagDetector::ExtractEdges(const FloatImage &im_mag,
                                       const FloatImage &im_theta) const {
  const int width = im_mag.width();
  const int height = im_mag.height();
  const size_t num_pixels = width * height;

  std::vector<Edge> edges;
  edges.reserve(num_pixels);

  // Bounds on the thetas assigned to this group. Note that because
  // theta is periodic, these are defined such that the average
  // value is contained *within* the interval.
  // limit scope of storage
  // Previously all this was on the stack, but this is 1.2MB for 320x240
  // images That's already a problem for OS X (default 512KB thread stack
  // size), could be a problem elsewhere for bigger images... so store on heap

  // do all the memory in one big block, exception safe
  std::vector<Stats> stats(num_pixels);

  for (int y = 0; y < height - 1; ++y) {
    for (int x = 0; x < width - 1; ++x) {
      const auto mag0 = im_mag.get(x, y);
      if (mag0 > Edge::kMinMag) {
        const int pid = y * width + x;
        stats[pid].mmin = mag0;
        stats[pid].mmax = mag0;

        const auto theta0 = im_theta.get(x, y);
        stats[pid].tmin = theta0;
        stats[pid].tmax = theta0;

        // Calculates then adds edges to 'vector<Edge> edges'
        const auto local_edges = CalcLocalEdges(x, y, im_mag, im_theta);
        edges.insert(edges.end(), local_edges.begin(), local_edges.end());
      }
    }
  }

  // std::sort is faster than std::stable_sort
  std::sort(edges.begin(), edges.end());

  DisjointSets dsets(num_pixels);
  MergeEdges(edges, dsets, stats, Edge::kMagThresh, Edge::kThetaThresh);

  return dsets;
}

TagDetector::Clusters
TagDetector::ClusterPixels(DisjointSets &dsets,
                           const FloatImage &im_mag) const {
  const int height = im_mag.height();
  const int width = im_mag.width();

  Clusters clusters;

  // TODO: if we refactored disjoint sets, then we can just iterate over each
  // set that is long enough instead of interate over every pixel
  // Then clusters can be refactored to be a vector<vector<XYW>>
  for (int y = 0; y < height - 1; ++y) {
    for (int x = 0; x < width - 1; ++x) {
      const int pid = y * width + x;

      const int rep = dsets.Find(pid);
      if (dsets.GetSetSize(rep) > Segment::kMinSegmentPixels) {
        const auto mag = im_mag.get(pid);
        const cv::Point3f xyw(x, y, mag);
        auto it = clusters.find(rep);
        if (it == clusters.end()) {
          // Spawn a new cluster
          clusters.insert({rep, {xyw}});
        } else {
          // Append to an existing cluster
          std::vector<cv::Point3f> &points = it->second;
          points.push_back(xyw);
        }
      }
    }
  }

  return clusters;
}

std::vector<Segment> TagDetector::FitLines(const Clusters &clusters,
                                           const FloatImage &im_mag,
                                           const FloatImage &im_theta) const {
  std::vector<Segment> segments;

  for (const auto &i_xyw : clusters) {
    const std::vector<cv::Point3f> &xyws = i_xyw.second;
    const auto lseg = LineSegment2D::LsqFitXyw(xyws);

    // filter short lines
    const auto length = Distance2D(lseg.p0(), lseg.p1());
    if (length > Segment::kMinLineLength) {
      Segment seg;
      const auto dy = lseg.p1().y - lseg.p0().y;
      const auto dx = lseg.p1().x - lseg.p0().x;

      const float tmp_theta = std::atan2(dy, dx);

      seg.set_theta(tmp_theta);
      seg.set_length(length);

      // We add an extra semantic to segments: the vector
      // p1->p2 will have dark on the left, white on the right.
      // To do this, we'll look at every gradient and each one
      // will vote for which way they think the gradient should
      // go. This is way more retentive than necessary: we
      // could probably sample just one point!

      float flip = 0, noflip = 0;
      for (const cv::Point3f &xyw : xyws) {
        const auto theta = im_theta.get(xyw.x, xyw.y);
        const auto mag = im_mag.get(xyw.x, xyw.y);

        // err *should* be +M_PI/2 for the correct winding, but if we
        // got the wrong winding, it'll be around -M_PI/2.
        float err = Mod2Pi(theta - seg.theta());

        if (err < 0) {
          noflip += mag;
        } else {
          flip += mag;
        }
      }

      if (flip > noflip) {
        seg.set_theta(seg.theta() + Pi<float>());
      }

      float dot = dx * std::cos(seg.theta()) + dy * std::sin(seg.theta());
      if (dot > 0) {
        seg.set_p0(lseg.p1());
        seg.set_p1(lseg.p0());
      } else {
        seg.set_p0(lseg.p0());
        seg.set_p1(lseg.p1());
      }

      segments.push_back(seg);
    }
  }
  return segments;
}

void TagDetector::ChainSegments(std::vector<Segment> &segments,
                                const FloatImage &image) const {
  const int width = image.width();
  const int height = image.height();
  Gridder<Segment> gridder(width, height, 10);

  // add every segment to the hash table according to the position of the
  // segment's first point. Remember that the first point has a specific meaning
  // due to our left-hand rule above.
  for (Segment &s : segments) {
    gridder.Add(s.x0(), s.y0(), &s);
  }

  // Now, find child segments that begin where each parent segment ends.
  for (Segment &parent_seg : segments) {
    // compute length of the line segment
    Line2D parent_line(parent_seg);

    auto iter = gridder.find(parent_seg.x1(), parent_seg.y1(),
                             0.5f * parent_seg.length());

    while (iter.hasNext()) {
      Segment &child_seg = iter.next();

      // We only look for segments that goes counterclockwise
      if (Mod2Pi(child_seg.theta() - parent_seg.theta()) > 0) {
        continue;
      }

      // compute intersection of points
      Line2D child_line(child_seg);

      auto p = parent_line.IntersectionWidth(child_line);
      if (p.x == -1) {
        continue;
      }

      float parent_dist = Distance2D(p, parent_seg.p1());
      float child_dist = Distance2D(p, child_seg.p0());

      if (std::max(parent_dist, child_dist) > parent_seg.length()) {
        continue;
      }

      // everything's OK, this child is a reasonable successor.
      parent_seg.children.push_back(&child_seg);
    }
  }
}

std::vector<Quad>
TagDetector::SearchQuads(std::vector<Segment> &segments) const {
  std::vector<Quad> quads;

  // TODO: need to look at this
  std::vector<Segment *> tmp(5);
  for (size_t i = 0; i < segments.size(); i++) {
    tmp[0] = &segments[i];
    Quad::Search(tmp, segments[i], 0, quads);
  }

  return quads;
}

std::vector<TagDetection>
TagDetector::DecodeQuads(const std::vector<Quad> &quads,
                         const FloatImage &image) const {
  std::vector<TagDetection> detections;

  for (const Quad &quad : quads) {
    const auto td = tag_family_.DecodeQuad(quad, image, black_border());
    if (td.good)
      detections.push_back(td);
  }

  return detections;
}

std::vector<TagDetection>
TagDetector::ResolveOverlap(const std::vector<TagDetection> &detections) const {
  std::vector<TagDetection> good_detections;
  good_detections.reserve(detections.size());

  for (const TagDetection &td : detections) {
    bool new_tag = true;

    for (TagDetection &gtd : good_detections) {
      // Tags have different id and they don't overlap too much
      if (td.id != gtd.id || !td.OverlapsTooMuch(gtd)) {
        continue;
      }

      // There's a conflict. We must pick one to keep.
      new_tag = false;

      // This detection is worse than the overlap one... just don't use it.
      if (td.hamming_distance > gtd.hamming_distance) {
        continue;
      }

      // Otherwise, keep the new one if it either has strictly *lower* error, or
      // greater perimeter.
      if (td.hamming_distance < gtd.hamming_distance ||
          td.obs_perimeter > gtd.obs_perimeter)
        gtd = td;
    }

    if (new_tag) {
      good_detections.push_back(td);
    }
  }

  return good_detections;
}

std::vector<TagDetection> TagDetector::ExtractTags(const cv::Mat &image) const {
  FloatImage im_orig(image);

  // ===========================================================================
  // Step 1: Preprocess image
  // ===========================================================================
  // Small Gaussian blur

  FloatImage im_decode, im_segment;
  Preprocess(im_orig, im_decode, im_segment);

  // ===========================================================================
  // Step 2: Compute local gradients and store magnitude and direction
  // ===========================================================================
  // This step is quite sensitive to noise, since a few bad theta estimates will
  // break up segments, causing us to miss Quads. It is useful to do a Gaussian
  // low pass on this step even if we don't want if for encoding.
  // NOTE: in Kaess' original code, mag is Ix^2 + Iy^2, so we need to modify
  // Edge::kMinMag to reflect this change accordingly

  FloatImage im_mag, im_theta;
  CalcPolar(im_segment, im_mag, im_theta);

  //============================================================================
  // Step 3: Extract edges by grouping pixels with similar thetas together.
  //============================================================================
  // This is a greedy algorithm: we start with the most similar pixels. We use
  // 4-connectivity.
  // NOTE: This is the most time consuming part!

  auto dsets = ExtractEdges(im_mag, im_theta);

  // ===========================================================================
  // Step 4: Loop over pixels to collect stattistics for each cluster
  // ===========================================================================
  // We will soon fit lines (segments) to these points.

  const auto clusters = ClusterPixels(dsets, im_mag);

  // ===========================================================================
  // Step 5: Loop over the clusters, fitting lines (which we call Segments).
  // ===========================================================================

  auto segments = FitLines(clusters, im_mag, im_theta);

  // ===========================================================================
  // Step 6: For each segment, find segments that begin where this segment ends.
  // ===========================================================================
  // We will chain segments together next. The gridder accelerates the search by
  // building (essentially) a 2D hash table.

  ChainSegments(segments, image);

  //============================================================================
  // Step 7: Search all connected segments for quads.
  //============================================================================
  // To see if any form a loop of length 4.

  const auto quads = SearchQuads(segments);

  //============================================================================
  // Step 8: Decode the quads.
  //============================================================================
  // For each quad, we first estimate a threshold color to decide between 0 and
  // 1. Then, we read off the bits and see if they make sense.

  const auto tags = DecodeQuads(quads, im_decode);

  // ===========================================================================
  // Step 9: Resolve overlapped tags.
  // ===========================================================================
  // Some quads may be detected more than once, due to
  // partial occlusion and our aggressive attempts to recover from
  // broken lines. When two quads (with the same id) overlap, we will
  // keep the one with the lowest error, and if the error is the same,
  // the one with the greatest observed perimeter.
  // NOTE: allow multiple non-overlapping detections of the same target.

  const auto good_tags = ResolveOverlap(tags);

  return good_tags;
}

void ConvertToGray(cv::InputArray in, cv::OutputArray out) {
  CV_Assert(in.getMat().channels() == 1 || in.getMat().channels() == 3);

  out.create(in.getMat().size(), CV_8UC1);
  if (in.getMat().type() == CV_8UC3) {
    cv::cvtColor(in.getMat(), out.getMat(), cv::COLOR_BGR2GRAY);
  } else {
    in.getMat().copyTo(out);
  }
}

std::vector<int> IndexFromNonZero(const cv::Mat mat) {
  const auto num_pixels = mat.total();
  const auto width = mat.cols;
  const auto height = mat.rows;
  std::vector<int> pids;
  pids.reserve(num_pixels / 2);

  for (int y = 0; y < height; ++y) {
    const auto up = mat.ptr<uchar>(y);
    for (int x = 0; x < width; ++x) {
      const auto e = up[x];
      if (e) {
        const auto pid = y * width + x;
        pids.push_back(pid);
      }
    }
  }
  return pids;
}

} // namespace AprilTags
