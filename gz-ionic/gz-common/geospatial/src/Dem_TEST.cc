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
#include <limits>
#include <gz/math/Angle.hh>
#include <gz/math/Vector3.hh>

#include "gz/common/geospatial/Dem.hh"

#include "gz/common/testing/AutoLogFixture.hh"
#include "gz/common/testing/TestPaths.hh"

using namespace gz;

class DemTest : public common::testing::AutoLogFixture { };

/////////////////////////////////////////////////
TEST_F(DemTest, MissingFile)
{
  common::Dem dem;
  EXPECT_NE(dem.Load("/file/shouldn/never/exist.png"), 0);
}

/////////////////////////////////////////////////
TEST_F(DemTest, NotDem)
{
  common::Dem dem;
  const auto path = common::testing::TestFile("CMakeLists.txt");
  EXPECT_NE(dem.Load(path), 0);
}

/////////////////////////////////////////////////
TEST_F(DemTest, UnsupportedDem)
{
  common::Dem dem;
  const auto path = common::testing::TestFile("data", "heightmap_bowl.png");
  EXPECT_NE(dem.Load(path), 0);
}

/////////////////////////////////////////////////
TEST_F(DemTest, NonSquaredDemPortrait)
{
  common::Dem dem;
  const auto path = common::testing::TestFile("data", "dem_portrait.tif");
  EXPECT_EQ(dem.Load(path), 0);
  EXPECT_DOUBLE_EQ(dem.Width(), dem.Height());
  EXPECT_DOUBLE_EQ(257, dem.Height());
  EXPECT_DOUBLE_EQ(257, dem.Width());
  EXPECT_DOUBLE_EQ(111565.57640012962, dem.WorldHeight());
  EXPECT_DOUBLE_EQ(87912.450080798269, dem.WorldWidth());
  EXPECT_DOUBLE_EQ(-6.2633352279663086, dem.MinElevation());
  EXPECT_DOUBLE_EQ(920.762939453125, dem.MaxElevation());
}

/////////////////////////////////////////////////
TEST_F(DemTest, NonSquaredDemLandscape)
{
  common::Dem dem;
  const auto path = common::testing::TestFile("data", "dem_landscape.tif");
  EXPECT_EQ(dem.Load(path), 0);
  EXPECT_DOUBLE_EQ(dem.Width(), dem.Height());
  EXPECT_DOUBLE_EQ(257, dem.Height());
  EXPECT_DOUBLE_EQ(257, dem.Width());
  EXPECT_DOUBLE_EQ(111565.57640012962, dem.WorldHeight());
  EXPECT_DOUBLE_EQ(87912.450080798269, dem.WorldWidth());
  EXPECT_DOUBLE_EQ(-4.7324686050415039, dem.MinElevation());
  EXPECT_DOUBLE_EQ(921.4481201171875, dem.MaxElevation());
}

/////////////////////////////////////////////////
TEST_F(DemTest, SquaredDem)
{
  common::Dem dem;
  const auto path = common::testing::TestFile("data", "dem_squared.tif");
  EXPECT_EQ(dem.Load(path), 0);
}

/////////////////////////////////////////////////
TEST_F(DemTest, BasicAPI)
{
  common::Dem dem;
  const auto path = common::testing::TestFile("data", "dem_squared.tif");
  EXPECT_EQ(dem.Load(path), 0);

  // Check filename
  EXPECT_EQ(path, dem.Filename());

  // Check the heights and widths
  EXPECT_EQ(129, static_cast<int>(dem.Height()));
  EXPECT_EQ(129, static_cast<int>(dem.Width()));
  EXPECT_FLOAT_EQ(3984.4849f, dem.WorldHeight());
  EXPECT_FLOAT_EQ(3139.7456f, dem.WorldWidth());
  EXPECT_FLOAT_EQ(65.3583f, dem.MinElevation());
  EXPECT_FLOAT_EQ(318.441f, dem.MaxElevation());

  // Check Elevation()
  unsigned int width = dem.Width();
  unsigned int height = dem.Height();
  EXPECT_FLOAT_EQ(215.82324f, dem.Elevation(0, 0));
  EXPECT_FLOAT_EQ(216.04961f, dem.Elevation(width - 1, 0));
  EXPECT_FLOAT_EQ(142.2274f, dem.Elevation(0, height - 1));
  EXPECT_FLOAT_EQ(209.14784f, dem.Elevation(width - 1, height - 1));

  // Illegal coordinates
  double inf = std::numeric_limits<double>::infinity();
  EXPECT_DOUBLE_EQ(inf, dem.Elevation(0, height));
  EXPECT_DOUBLE_EQ(inf, dem.Elevation(width, 0));
  EXPECT_DOUBLE_EQ(inf, dem.Elevation(width, height));

  // Check GeoReferenceOrigin()
  gz::math::Angle latitude, longitude;
  EXPECT_TRUE(dem.GeoReferenceOrigin(latitude, longitude));
  EXPECT_FLOAT_EQ(38.001667f, latitude.Degree());
  EXPECT_FLOAT_EQ(-122.22278f, longitude.Degree());

  // Emulate Earth as a custom surface.
  common::Dem demCustomSurface;
  auto earthSc = math::SphericalCoordinates();
  auto customSc = math::SphericalCoordinates(
      math::SphericalCoordinates::CUSTOM_SURFACE,
      earthSc.SurfaceRadius(),
      earthSc.SurfaceRadius());
  demCustomSurface.SetSphericalCoordinates(customSc);
  EXPECT_EQ(demCustomSurface.Load(path), 0);
  EXPECT_FLOAT_EQ(3984.4849f, demCustomSurface.WorldHeight());
  EXPECT_FLOAT_EQ(3139.7456f, demCustomSurface.WorldWidth());
}

/////////////////////////////////////////////////
TEST_F(DemTest, FillHeightmap)
{
  common::Dem dem;
  const auto path = common::testing::TestFile("data", "dem_squared.tif");
  EXPECT_EQ(dem.Load(path), 0);

  // Use FillHeightMap() to retrieve a vector<float> after some transformations
  int subsampling;
  unsigned vertSize;
  gz::math::Vector3d size;
  gz::math::Vector3d scale;
  bool flipY;
  std::vector<float> elevations;

  subsampling = 2;
  vertSize = (dem.Width() * subsampling) - 1;
  size.X(dem.WorldWidth());
  size.Y(dem.WorldHeight());
  size.Z(dem.MaxElevation() - dem.MinElevation());
  scale.X(size.X() / vertSize);
  scale.Y(size.Y() / vertSize);

  if (gz::math::equal(dem.MaxElevation(), 0.0f))
    scale.Z(fabs(size.Z()));
  else
    scale.Z(fabs(size.Z()) / dem.MaxElevation());
  flipY = false;

  dem.FillHeightMap(subsampling, vertSize, size, scale, flipY, elevations);

  // Check the size of the returned vector
  EXPECT_EQ(vertSize * vertSize, elevations.size());

  // Check the elevation of some control points
  EXPECT_FLOAT_EQ(184.94113f, elevations.at(0));
  EXPECT_FLOAT_EQ(179.63583f, elevations.at(elevations.size() - 1));
  EXPECT_FLOAT_EQ(213.42966f, elevations.at(elevations.size() / 2));
}

/////////////////////////////////////////////////
TEST_F(DemTest, UnfinishedDem)
{
  common::Dem dem;
  auto path = common::testing::TestFile("data", "dem_unfinished.tif");
  EXPECT_EQ(dem.Load(path), 0);

  // Check that the min and max elevations are valid for an unfinished
  // and unfilled dem.
  EXPECT_EQ(33, static_cast<int>(dem.Height()));
  EXPECT_EQ(33, static_cast<int>(dem.Width()));
  EXPECT_FLOAT_EQ(111287.59f, dem.WorldHeight());
  EXPECT_FLOAT_EQ(88878.297f, dem.WorldWidth());
  EXPECT_FLOAT_EQ(-32768.0f, dem.MinElevation());
  EXPECT_FLOAT_EQ(1909.0f, dem.MaxElevation());

  // test another dem file with multiple nodata values
  common::Dem demNoData;

  path = common::testing::TestFile("data", "dem_nodata.dem");
  EXPECT_EQ(demNoData.Load(path), 0);

  // Check that the min and max elevations are valid for a dem with multiple
  // nodata values
  EXPECT_EQ(65, static_cast<int>(demNoData.Height()));
  EXPECT_EQ(65, static_cast<int>(demNoData.Width()));

  // relaxed tolerances for macOS & windows
  EXPECT_NEAR(7499.8281, demNoData.WorldHeight(), 0.1);
  EXPECT_NEAR(14150.225, demNoData.WorldWidth(), 0.1);

  // -32767 is the nodata value, so it's ignored when computing the min
  // elevation
  EXPECT_FLOAT_EQ(682.0f, demNoData.MinElevation());
  EXPECT_FLOAT_EQ(2932.0f, demNoData.MaxElevation());
}

/////////////////////////////////////////////////
TEST_F(DemTest, NaNNoData)
{
  common::Dem dem;
  auto path = common::testing::TestFile("data", "dem_nodata_nan.nc");
  EXPECT_EQ(dem.Load(path), 0);

  // Check that the min and max elevations are valid for a DEM with NaN
  // nodata values
  EXPECT_EQ(129, static_cast<int>(dem.Height()));
  EXPECT_EQ(129, static_cast<int>(dem.Width()));

  EXPECT_NEAR(7464.7589424555326, dem.WorldHeight(), 0.1);
  EXPECT_NEAR(14244.280980717675, dem.WorldWidth(), 0.1);

  EXPECT_FLOAT_EQ(682.0f, dem.MinElevation());
  EXPECT_FLOAT_EQ(2932.0f, dem.MaxElevation());
}

/////////////////////////////////////////////////
TEST_F(DemTest, UnknownDem)
{
  // moon
  common::Dem dem;
  auto path = common::testing::TestFile("data", "dem_moon.tif");
  EXPECT_EQ(dem.Load(path), 0);

  EXPECT_EQ(33, static_cast<int>(dem.Height()));
  EXPECT_EQ(33, static_cast<int>(dem.Width()));
  EXPECT_FLOAT_EQ(-212.29616f, dem.MinElevation());
  EXPECT_FLOAT_EQ(-205.44009f, dem.MaxElevation());

  // unable to determne world width/height of non earth DEM
  EXPECT_FLOAT_EQ(-1, dem.WorldHeight());
  EXPECT_FLOAT_EQ(-1, dem.WorldWidth());

  // unable to get coordinates in WGS84
  gz::math::Angle latitude, longitude;
  EXPECT_FALSE(dem.GeoReferenceOrigin(latitude, longitude));
}

TEST_F(DemTest, LunarDemLoad)
{
  // Load Moon DEM
  common::Dem dem;
  auto path = common::testing::TestFile("data", "dem_moon.tif");
  // Providing spherical coordinates object.
  auto moonSc = math::SphericalCoordinates(
      math::SphericalCoordinates::MOON_SCS);
  dem.SetSphericalCoordinates(moonSc);
  EXPECT_EQ(dem.Load(path), 0);
  EXPECT_NEAR(dem.WorldWidth(), 80.0417, 1e-2);
  EXPECT_NEAR(dem.WorldHeight(), 80.0417, 1e-2);

  // Use custom spherical coordinates object with same axes as the moon.
  auto customSc = math::SphericalCoordinates(
      math::SphericalCoordinates::CUSTOM_SURFACE,
      moonSc.SurfaceAxisEquatorial(),
      moonSc.SurfaceAxisPolar());
  dem.SetSphericalCoordinates(customSc);
  EXPECT_EQ(dem.Load(path), 0);
  EXPECT_NEAR(dem.WorldWidth(), 80.0417, 1e-2);
  EXPECT_NEAR(dem.WorldHeight(), 80.0417, 1e-2);
}

TEST_F(DemTest, LargeVRTWithLimits)
{
  // Load a large VRT DEM using GDAL but set limits on the size.
  common::Dem dem;
  dem.SetRasterXSizeLimit(100);
  dem.SetRasterYSizeLimit(50);
  auto const path = common::testing::TestFile("data", "ap_srtm1.vrt");
  auto const res = dem.Load(path);
  EXPECT_EQ(res, 0);
  EXPECT_EQ(dem.RasterXSizeLimit(), 100);
  EXPECT_EQ(dem.RasterYSizeLimit(), 50);
}

TEST_F(DemTest, LargeVRTWithVSIZIPAndLimits)
{
  // Load a large vzizip VRT DEM using GDAL but set limits on the size.
  common::Dem dem;
  dem.SetRasterXSizeLimit(100);
  dem.SetRasterYSizeLimit(50);
  auto const path = common::testing::TestFile("data", "ap_srtm1.zip");
  auto const res = dem.Load("/vsizip/" + path + "/ap_srtm1.vrt");
  EXPECT_EQ(res, 0);
}

TEST_F(DemTest, LargeVRTWithoutLimitsThrows)
{
  GTEST_SKIP() << "Skipping this test because it's not platform-portable";
  // Load a large VRT DEM without limits.
  common::Dem dem;
  auto const path = common::testing::TestFile("data", "ap_srtm1.vrt");
  // We expect not to be able to allocate the data,
  // so ensure we throw instead of segfault.
  EXPECT_THROW(dem.Load(path), std::bad_alloc);
}
