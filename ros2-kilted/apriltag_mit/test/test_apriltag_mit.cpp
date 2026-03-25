#include "apriltag_mit/apriltag_mit.h"

#include <gtest/gtest.h>
#include <opencv2/highgui/highgui.hpp>
#include <ros/package.h>

using namespace apriltag_mit;
using testing::Test;
using testing::WithParamInterface;
using testing::Values;

class SampleImageTest : public Test {
protected:
  SampleImageTest()
      : package_name_("apriltag_mit"),
        package_path_(ros::package::getPath(package_name_)),
        image_path_(package_path_ + "/image/tag_sampler.png"),
        test_image_(cv::imread(image_path_, CV_LOAD_IMAGE_GRAYSCALE)) {}

  std::string package_name_, package_path_, image_path_;
  cv::Mat test_image_;
};

class TagFamilyTest : public SampleImageTest,
                      public WithParamInterface<TagCodes> {
public:
  TagFamilyTest() : tag_detector_(GetParam()) {}

protected:
  TagDetector tag_detector_;
};

TEST_P(TagFamilyTest, Detection) {
  const auto tag_detection = tag_detector_.ExtractTags(test_image_);
  EXPECT_EQ(4, tag_detection.size());
}

INSTANTIATE_TEST_CASE_P(ThreeTagFamilies, TagFamilyTest,
                        Values(tag_codes_36h11, tag_codes_25h9,
                               tag_codes_16h5));

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
