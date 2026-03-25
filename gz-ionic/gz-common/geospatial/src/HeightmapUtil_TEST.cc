/*
 * Copyright (C) 2025 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#include <gtest/gtest.h>
#include <memory>

#include "gz/common/geospatial/HeightmapUtil.hh"

#include "gz/common/testing/AutoLogFixture.hh"
#include "gz/common/testing/TestPaths.hh"

using namespace gz;

class HeightmapLoaderTest : public common::testing::AutoLogFixture { };

/////////////////////////////////////////////////
TEST_F(HeightmapLoaderTest, SupportedImageHeightmapFileExtension)
{
  EXPECT_TRUE(common::isSupportedImageHeightmapFileExtension("f.jpg"));
  EXPECT_TRUE(common::isSupportedImageHeightmapFileExtension("f.jpeg"));
  EXPECT_TRUE(common::isSupportedImageHeightmapFileExtension("f.png"));
  EXPECT_TRUE(common::isSupportedImageHeightmapFileExtension("f.JPG"));
  EXPECT_TRUE(common::isSupportedImageHeightmapFileExtension("f.JPEG"));
  EXPECT_TRUE(common::isSupportedImageHeightmapFileExtension("f.PNG"));
  EXPECT_TRUE(common::isSupportedImageHeightmapFileExtension(".PNG"));
  EXPECT_FALSE(common::isSupportedImageHeightmapFileExtension("f.tiff"));
  EXPECT_FALSE(common::isSupportedImageHeightmapFileExtension("invalid"));
  EXPECT_FALSE(common::isSupportedImageHeightmapFileExtension(""));
}

/////////////////////////////////////////////////
TEST_F(HeightmapLoaderTest, LoadImage)
{
  const auto path = common::testing::TestFile("data", "heightmap_bowl.png");
  std::unique_ptr<common::HeightmapData> data =
      common::loadHeightmapData(path);
  EXPECT_NE(nullptr, data);
}

#ifndef BAZEL_DISABLE_DEM_LOADER
/////////////////////////////////////////////////
TEST_F(HeightmapLoaderTest, LoadDEM)
{
  const auto path = common::testing::TestFile("data", "dem_squared.tif");
  std::unique_ptr<common::HeightmapData> data =
      common::loadHeightmapData(path);
  EXPECT_NE(nullptr, data);
}
#endif

/////////////////////////////////////////////////
TEST_F(HeightmapLoaderTest, LoadInvalidFile)
{
  EXPECT_EQ(nullptr, common::loadHeightmapData("invalid/file.jpg"));
  EXPECT_EQ(nullptr, common::loadHeightmapData("invalid/file_no_extension"));
}
