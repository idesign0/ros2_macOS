#include "apriltag_mit/AprilTags/Quad.h"
#include "apriltag_mit/AprilTags/Line2D.h"
#include "apriltag_mit/AprilTags/MathUtil.h"
#include "apriltag_mit/AprilTags/Segment.h"
#include <opencv2/calib3d/calib3d.hpp>

namespace AprilTags {

Quad::Quad(const std::vector<cv::Point2f> &p)
    : p(p), segments(), obs_perimeter(), p0_(p[0]), p3_(p[3]),
      p01_(p[1] - p[0]), p32_(p[2] - p[3]) {}

cv::Point2f Quad::Interpolate(const cv::Point2f &p) const {
  const float kx = (p.x + 1) / 2;
  const float ky = (p.y + 1) / 2;
  const auto r1 = p0_ + p01_ * kx;
  const auto r2 = p3_ + p32_ * kx;
  const auto r = r1 + (r2 - r1) * ky;
  return r;
}

cv::Point2f Quad::Interpolate01(const cv::Point2f &p) const {
  return Interpolate(2 * p - cv::Point2f(1, 1));
}

GrayModel Quad::MakeGrayModel(const FloatImage &image,
                              unsigned length_bits) const {
  GrayModel model;
  const int lb = length_bits;

  // Only need to loop through the boundary
  for (int yb = -1; yb <= lb; ++yb) {
    // Convert to normalized coordinates 01
    const float yn = (yb + 0.5f) / lb;
    for (int xb = -1; xb <= lb; ++xb) {
      // Skip if inside quad boundary
      if (IsInsideInnerBorder(xb, yb, lb))
        continue;

      const float xn = (xb + 0.5f) / lb;
      // Convert to image coordinates
      const auto pi = Interpolate01({xn, yn});
      int xi = pi.x + 0.5;
      int yi = pi.y + 0.5;

      // Skip if outside image
      if (!IsInsideImage(xi, yi, image))
        continue;

      const float v = image.get(xi, yi);
      if (IsOnOuterBorder(xb, yb, lb)) {
        model.AddWhiteObs(xn, yn, v);
      } else if (IsOnInnerBorder(xb, yb, lb)) {
        model.AddBlackObs(xn, yn, v);
      }
    }
  }

  // Don't forget to Fit the model
  model.Fit();
  return model;
}

code_t Quad::DecodePayload(const FloatImage &image, const GrayModel &model,
                           unsigned dimension_bits,
                           unsigned black_border) const {
  code_t code = 0;
  const int lb = 2 * black_border + dimension_bits;

  for (int yb = dimension_bits - 1; yb >= 0; yb--) {
    float yn = (black_border + yb + 0.5f) / lb;
    for (int xb = 0; xb < dimension_bits; xb++) {
      float xn = (black_border + xb + 0.5f) / lb;

      const auto pi = Interpolate01({xn, yn});
      int xi = pi.x + 0.5;
      int yi = pi.y + 0.5;

      if (!IsInsideImage(xi, yi, image)) {
        return 0;
      }

      const float threshold = model.CalcThreshold(xn, yn);
      float v = image.get(xi, yi);
      code = code << 1;
      if (v > threshold) {
        code |= 1;
      }
    }
  }
  return code;
}

code_t Quad::ToTagCode(const FloatImage &image, unsigned dimension_bits,
                       unsigned black_border) const {
  const int lb = 2 * black_border + dimension_bits;
  const auto model = MakeGrayModel(image, lb);
  return DecodePayload(image, model, dimension_bits, black_border);
}

void Quad::Search(std::vector<Segment *> &path, Segment &parent, int depth,
                  std::vector<Quad> &quads) {
  // terminal depth occurs when we've found four segments.
  if (depth == 4) {
    // cout << "Entered terminal depth" << endl; // debug code

    // Is the first segment the same as the last segment (i.e., a loop?)
    if (path[4] == path[0]) {
      // the 4 corners of the quad as computed by the intersection of segments.
      std::vector<cv::Point2f> p(4);
      float calc_perimeter = 0;
      bool bad = false;
      for (size_t i = 0; i < 4; i++) {
        // compute intersections between all the lines. This will give us
        // sub-pixel accuracy for the corners of the quad.
        Line2D line_a(*path[i]);
        Line2D line_b(*path[i + 1]);

        p[i] = line_a.IntersectionWidth(line_b);
        calc_perimeter += path[i]->length();

        // no intersection? Occurs when the lines are almost parallel.
        if (p[i].x == -1) {
          bad = true;
        }
      }
      // cout << "bad = " << bad << endl;
      // eliminate quads that don't form a simply connected loop, i.e., those
      // that form an hour glass, or wind the wrong way.
      if (!bad) {
        float t0 = std::atan2(p[1].y - p[0].y, p[1].x - p[0].x);
        float t1 = std::atan2(p[2].y - p[1].y, p[2].x - p[1].x);
        float t2 = std::atan2(p[3].y - p[2].y, p[3].x - p[2].x);
        float t3 = std::atan2(p[0].y - p[3].y, p[0].x - p[3].x);

        //  double ttheta = fmod(t1-t0, 2*M_PI) + fmod(t2-t1, 2*M_PI) +
        //    fmod(t3-t2, 2*M_PI) + fmod(t0-t3, 2*M_PI);
        float total_theta = Mod2Pi(t1 - t0) + Mod2Pi(t2 - t1) +
                            Mod2Pi(t3 - t2) + Mod2Pi(t0 - t3);
        // cout << "ttheta=" << ttheta << endl;
        // the magic value is -2*PI. It should be exact,
        // but we allow for (lots of) numeric imprecision.
        if (total_theta < -7 || total_theta > -5)
          bad = true;
      }

      if (!bad) {
        float d0 = Distance2D(p[0], p[1]);
        float d1 = Distance2D(p[1], p[2]);
        float d2 = Distance2D(p[2], p[3]);
        float d3 = Distance2D(p[3], p[0]);
        float d4 = Distance2D(p[0], p[2]);
        float d5 = Distance2D(p[1], p[3]);

        // check sizes
        if (d0 < Quad::kMinEdgeLength || d1 < Quad::kMinEdgeLength ||
            d2 < Quad::kMinEdgeLength || d3 < Quad::kMinEdgeLength ||
            d4 < Quad::kMinEdgeLength || d5 < Quad::kMinEdgeLength) {
          bad = true;
        }

        // check aspect ratio
        float dmax = std::max(std::max(d0, d1), std::max(d2, d3));
        float dmin = std::min(std::min(d0, d1), std::min(d2, d3));

        if (dmax > dmin * Quad::kMaxQuadAspectRatio) {
          bad = true;
        }
      }

      if (!bad) {
        Quad quad(p);
        quad.segments = path;
        quad.obs_perimeter = calc_perimeter;
        quads.push_back(quad);
      }
    }
    return;
  }

  // Not terminal depth. Recurse on any children that obey the correct
  // handedness.
  for (unsigned int i = 0; i < parent.children.size(); i++) {
    Segment &child = *parent.children[i];
    // (handedness was checked when we created the children)

    // we could rediscover each quad 4 times (starting from
    // each corner). If we had an arbitrary ordering over
    // points, we can eliminate the redundant detections by
    // requiring that the first corner have the lowest
    // value. We're arbitrarily going to use theta...
    if (child.theta() > path[0]->theta()) {
      continue;
    }
    path[depth + 1] = &child;
    Search(path, child, depth + 1, quads);
  }
}

} // namespace AprilTags
