#include "apriltag_mit/AprilTags/Segment.h"

namespace AprilTags {

Segment::Segment() : theta_(0), length_(0), id_(++counter) {}

int Segment::counter = 0;

} // namespace
