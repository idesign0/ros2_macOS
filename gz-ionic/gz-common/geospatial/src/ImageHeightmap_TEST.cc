/*
 * Copyright (C) 2016 Open Source Robotics Foundation
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

#include "gz/common/geospatial/ImageHeightmap.hh"

#include "gz/common/testing/AutoLogFixture.hh"
#include "gz/common/testing/TestPaths.hh"

#define ELEVATION_TOL 1e-8

using namespace gz;

class ImageHeightmapTest : public common::testing::AutoLogFixture { };

class DemTest : public common::testing::AutoLogFixture { };

/////////////////////////////////////////////////
TEST_F(DemTest, MissingFile)
{
  common::ImageHeightmap img;
  EXPECT_EQ(-1, img.Load("/file/shouldn/never/exist.png"));
}

/////////////////////////////////////////////////
TEST_F(DemTest, NotImage)
{
  common::ImageHeightmap img;
  const auto path = common::testing::TestFile("data", "dem_portrait.tif");
  EXPECT_EQ(-1, img.Load(path));
}

/////////////////////////////////////////////////
TEST_F(ImageHeightmapTest, BasicAPI)
{
  common::ImageHeightmap img;

  const auto path = common::testing::TestFile("data", "heightmap_bowl.png");
  std::cout << "PATH[" << path << "]\n";
  EXPECT_EQ(0, img.Load(path));

  // Check filename
  EXPECT_EQ(path, img.Filename());

  // Check the heights and widths
  EXPECT_EQ(129, static_cast<int>(img.Height()));
  EXPECT_EQ(129, static_cast<int>(img.Width()));
  EXPECT_NEAR(0.99607843, img.MaxElevation(), ELEVATION_TOL);
}

/////////////////////////////////////////////////
TEST_F(ImageHeightmapTest, FillHeightmap)
{
  common::ImageHeightmap img;

  const auto path = common::testing::TestFile("data", "heightmap_bowl.png");
  EXPECT_EQ(0, img.Load(path));

  // Use FillHeightMap() to retrieve a vector<float> after some transformations
  int subsampling;
  unsigned vertSize;
  math::Vector3d size;
  math::Vector3d scale;
  bool flipY;
  std::vector<float> elevations;

  subsampling = 2;
  vertSize = (img.Width() * subsampling) - 1;
  size.X(129);
  size.Y(129);
  size.Z(10);
  scale.X(size.X() / vertSize);
  scale.Y(size.Y() / vertSize);
  if (math::equal(img.MaxElevation(), 0.0f))
    scale.Z(fabs(size.Z()));
  else
    scale.Z(fabs(size.Z()) / img.MaxElevation());
  flipY = false;

  img.FillHeightMap(subsampling, vertSize, size, scale, flipY, elevations);

  // Check the size of the returned vector
  EXPECT_EQ(vertSize * vertSize, elevations.size());

  // Check the elevation of some control points
  EXPECT_NEAR(0.0, elevations.at(0), ELEVATION_TOL);
  EXPECT_NEAR(10.0, elevations.at(elevations.size() - 1), ELEVATION_TOL);
  EXPECT_NEAR(5.0, elevations.at(elevations.size() / 2), ELEVATION_TOL);
}
