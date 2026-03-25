#pragma once

#include "apriltag_mit/AprilTags/Tag16h5.h"
#include "apriltag_mit/AprilTags/Tag25h9.h"
#include "apriltag_mit/AprilTags/Tag36h11.h"
#include "apriltag_mit/AprilTags/TagDetection.h"
#include "apriltag_mit/AprilTags/TagDetector.h"

#include <boost/shared_ptr.hpp>

namespace AprilTags {

using TagDetectorPtr = boost::shared_ptr<TagDetector>;
using FloatPair = std::pair<float, float>;

} // namespace apriltag_mit

namespace apriltag_mit = AprilTags;
