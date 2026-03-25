/*
 * Copyright (C) 2012 Open Source Robotics Foundation
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

#include "gz/math/SphericalCoordinates.hh"

using namespace gz;

//////////////////////////////////////////////////
// Test different constructors, default parameters
TEST(SphericalCoordinatesTest, Constructor)
{
  // Default surface type
  math::SphericalCoordinates::SurfaceType st =
    math::SphericalCoordinates::EARTH_WGS84;

  // No arguments, default parameters
  {
    math::SphericalCoordinates sc;
    EXPECT_EQ(sc.Surface(), st);
    EXPECT_EQ(sc.LatitudeReference(), math::Angle());
    EXPECT_EQ(sc.LongitudeReference(), math::Angle());
    EXPECT_EQ(sc.HeadingOffset(), math::Angle());
    EXPECT_NEAR(sc.ElevationReference(), 0.0, 1e-6);
  }

  // SurfaceType argument, default parameters
  {
    math::SphericalCoordinates sc(st);
    EXPECT_EQ(sc.Surface(), st);
    EXPECT_EQ(sc.LatitudeReference(), math::Angle());
    EXPECT_EQ(sc.LongitudeReference(), math::Angle());
    EXPECT_EQ(sc.HeadingOffset(), math::Angle());
    EXPECT_NEAR(sc.ElevationReference(), 0.0, 1e-6);
  }

  // All arguments
  {
    math::Angle lat(0.3), lon(-1.2), heading(0.5);
    double elev = 354.1;
    math::SphericalCoordinates sc(st, lat, lon, elev, heading);
    EXPECT_EQ(sc.Surface(), st);
    EXPECT_EQ(sc.LatitudeReference(), lat);
    EXPECT_EQ(sc.LongitudeReference(), lon);
    EXPECT_EQ(sc.HeadingOffset(), heading);
    EXPECT_NEAR(sc.ElevationReference(), elev, 1e-6);

    // Copy constructor
    math::SphericalCoordinates sc2(sc);
    EXPECT_EQ(sc, sc2);
  }

  // Bad surface type, this should throw an error
  math::SphericalCoordinates invalidSC(
      static_cast<math::SphericalCoordinates::SurfaceType>(3));
  EXPECT_EQ(invalidSC.Surface(), 3);
}

//////////////////////////////////////////////////
// SurfaceType Convert function
TEST(SphericalCoordinatesTest, Convert)
{
  // Default surface type
  math::SphericalCoordinates::SurfaceType st =
    math::SphericalCoordinates::EARTH_WGS84;

  EXPECT_EQ(math::SphericalCoordinates::Convert("EARTH_WGS84"), st);

  EXPECT_EQ(math::SphericalCoordinates::EARTH_WGS84,
            math::SphericalCoordinates::Convert("OTHER-COORD"));

  EXPECT_EQ("EARTH_WGS84", math::SphericalCoordinates::Convert(st));
  EXPECT_EQ("EARTH_WGS84", math::SphericalCoordinates::Convert(
      static_cast<math::SphericalCoordinates::SurfaceType>(3)));

  // For the Moon surface type
  st = math::SphericalCoordinates::MOON_SCS;
  EXPECT_EQ(math::SphericalCoordinates::Convert("MOON_SCS"), st);
  EXPECT_EQ("MOON_SCS", math::SphericalCoordinates::Convert(st));

  // For the custom surface type
  st = math::SphericalCoordinates::CUSTOM_SURFACE;
  EXPECT_EQ(math::SphericalCoordinates::Convert("CUSTOM_SURFACE"), st);
  EXPECT_EQ("CUSTOM_SURFACE", math::SphericalCoordinates::Convert(st));
}

//////////////////////////////////////////////////
// Test Set functions
TEST(SphericalCoordinatesTest, SetFunctions)
{
  // Default surface type
  math::SphericalCoordinates::SurfaceType st =
    math::SphericalCoordinates::EARTH_WGS84;

  // Default parameters
  math::SphericalCoordinates sc;
  EXPECT_EQ(sc.Surface(), st);
  EXPECT_EQ(sc.LatitudeReference(), math::Angle());
  EXPECT_EQ(sc.LongitudeReference(), math::Angle());
  EXPECT_EQ(sc.HeadingOffset(), math::Angle());
  EXPECT_NEAR(sc.ElevationReference(), 0.0, 1e-6);
  EXPECT_NEAR(sc.SurfaceRadius(),
      6371000.0, 1e-3);
  EXPECT_NEAR(sc.SurfaceAxisEquatorial(),
      6378137.0, 1e-3);
  EXPECT_NEAR(sc.SurfaceAxisPolar(),
      6356752.314245, 1e-3);
  EXPECT_NEAR(sc.SurfaceFlattening(),
      1.0/298.257223563, 1e-5);

  {
    math::Angle lat(0.3), lon(-1.2), heading(0.5);
    double elev = 354.1;
    sc.SetSurface(st);
    sc.SetLatitudeReference(lat);
    sc.SetLongitudeReference(lon);
    sc.SetHeadingOffset(heading);
    sc.SetElevationReference(elev);

    EXPECT_EQ(sc.Surface(), st);
    EXPECT_EQ(sc.LatitudeReference(), lat);
    EXPECT_EQ(sc.LongitudeReference(), lon);
    EXPECT_EQ(sc.HeadingOffset(), heading);
    EXPECT_NEAR(sc.ElevationReference(), elev, 1e-6);
    EXPECT_NEAR(sc.SurfaceRadius(),
        6371000.0, 1e-3);
    EXPECT_NEAR(sc.SurfaceAxisEquatorial(),
        6378137.0, 1e-3);
    EXPECT_NEAR(sc.SurfaceAxisPolar(),
        6356752.314245, 1e-3);
    EXPECT_NEAR(sc.SurfaceFlattening(),
        1.0/298.257223563, 1e-5);
  }

  // Moon surface type
  st = math::SphericalCoordinates::MOON_SCS;
  math::SphericalCoordinates moonSC(st);
  moonSC.SetSurface(st);
  EXPECT_EQ(moonSC.Surface(), st);
  EXPECT_NEAR(moonSC.SurfaceRadius(),
      1737400.0, 1e-3);
  EXPECT_NEAR(moonSC.SurfaceAxisEquatorial(),
      1738100.0, 1e-3);
  EXPECT_NEAR(moonSC.SurfaceAxisPolar(),
      1736000.0, 1e-3);
  EXPECT_NEAR(moonSC.SurfaceFlattening(),
      0.0012, 1e-5);
}

//////////////////////////////////////////////////
/// Test invalid parameters for custom surface
TEST(SphericalCoordinatesTest, InvalidParameters)
{
  // Earth's constants
  double g_EarthWGS84AxisEquatorial = 6378137.0;
  double g_EarthWGS84AxisPolar = 6356752.314245;
  double g_EarthWGS84Flattening = 1.0/298.257223563;
  double g_EarthRadius = 6371000.0;

  // Create a custom surface with invalid parameters.
  math::SphericalCoordinates scInvalid(
      math::SphericalCoordinates::CUSTOM_SURFACE,
      -1, -1);

  // These should be rejected and default to Earth's
  // parameters.
  EXPECT_NEAR(scInvalid.SurfaceRadius(), g_EarthRadius,
      1e-3);
  EXPECT_NEAR(scInvalid.SurfaceAxisEquatorial(),
      g_EarthWGS84AxisEquatorial, 1e-3);
  EXPECT_NEAR(scInvalid.SurfaceAxisPolar(),
      g_EarthWGS84AxisPolar, 1e-3);
  EXPECT_NEAR(scInvalid.SurfaceFlattening(),
      g_EarthWGS84Flattening, 1e-3);

  // Create a custom surface with valid parameters.
  math::SphericalCoordinates scValid(
      math::SphericalCoordinates::CUSTOM_SURFACE,
      100, 100);

  // These should be accepted
  EXPECT_NEAR(scValid.SurfaceRadius(), 100,
      1e-3);
  EXPECT_NEAR(scValid.SurfaceAxisEquatorial(),
      100, 1e-3);
  EXPECT_NEAR(scValid.SurfaceAxisPolar(),
      100, 1e-3);
  EXPECT_NEAR(scValid.SurfaceFlattening(),
      0, 1e-3);
}

//////////////////////////////////////////////////
// Test coordinate transformations
TEST(SphericalCoordinatesTest, CoordinateTransforms)
{
  // Default surface type
  math::SphericalCoordinates::SurfaceType st =
    math::SphericalCoordinates::EARTH_WGS84;

  {
    // Parameters
    math::Angle lat(0.3), lon(-1.2),
      heading(math::Angle::HalfPi);
    double elev = 354.1;
    math::SphericalCoordinates sc(st, lat, lon, elev, heading);

    // Check GlobalFromLocal with heading offset of 90 degrees
    // Heading 0:  X == East, Y == North, Z == Up
    // Heading 90: X == North, Y == West , Z == Up
    {
      // local frame
      math::CoordinateVector3 xyz;
      // east, north, up
      std::optional<math::CoordinateVector3> enuOpt;
      std::optional<math::CoordinateVector3> xyzOpt;

      xyz.SetMetric(1, 0, 0);
      enuOpt = sc.GlobalFromLocalVelocity(xyz);
      ASSERT_TRUE(enuOpt.has_value());
      ASSERT_TRUE(enuOpt->IsMetric());
      EXPECT_NEAR(*enuOpt->Y()/*North*/, *xyz.X(), 1e-6);
      EXPECT_NEAR(*enuOpt->X()/*East*/, -*xyz.Y(), 1e-6);
      xyzOpt = sc.LocalFromGlobalVelocity(*enuOpt);
      ASSERT_TRUE(xyzOpt.has_value());
      EXPECT_EQ(xyz, *xyzOpt);

      xyz.SetMetric(0, 1, 0);
      enuOpt = sc.GlobalFromLocalVelocity(xyz);
      ASSERT_TRUE(enuOpt.has_value());
      ASSERT_TRUE(enuOpt->IsMetric());
      EXPECT_NEAR(*enuOpt->Y(), *xyz.X(), 1e-6);
      EXPECT_NEAR(*enuOpt->X(), -*xyz.Y(), 1e-6);
      xyzOpt = sc.LocalFromGlobalVelocity(*enuOpt);
      ASSERT_TRUE(xyzOpt.has_value());
      EXPECT_EQ(xyz, *xyzOpt);

      xyz.SetMetric(1, -1, 0);
      enuOpt = sc.GlobalFromLocalVelocity(xyz);
      ASSERT_TRUE(enuOpt.has_value());
      ASSERT_TRUE(enuOpt->IsMetric());
      EXPECT_NEAR(*enuOpt->Y(), *xyz.X(), 1e-6);
      EXPECT_NEAR(*enuOpt->X(), -*xyz.Y(), 1e-6);
      xyzOpt = sc.LocalFromGlobalVelocity(*enuOpt);
      ASSERT_TRUE(xyzOpt.has_value());
      EXPECT_EQ(xyz, *xyzOpt);

      xyz.SetMetric(2243.52334, 556.35, 435.6553);
      enuOpt = sc.GlobalFromLocalVelocity(xyz);
      ASSERT_TRUE(enuOpt.has_value());
      ASSERT_TRUE(enuOpt->IsMetric());
      EXPECT_NEAR(*enuOpt->Y(), *xyz.X(), 1e-6);
      EXPECT_NEAR(*enuOpt->X(), -*xyz.Y(), 1e-6);
      xyzOpt = sc.LocalFromGlobalVelocity(*enuOpt);
      ASSERT_TRUE(xyzOpt.has_value());
      EXPECT_EQ(xyz, *xyzOpt);
    }

    // Check SphericalFromLocal
    {
      // local frame
      math::CoordinateVector3 xyz;
      // spherical coordinates
      std::optional<math::CoordinateVector3> sphOpt;
      std::optional<math::CoordinateVector3> xyzOpt;

      // No offset
      xyz.SetMetric(0, 0, 0);
      sphOpt = sc.SphericalFromLocalPosition(xyz);
      ASSERT_TRUE(sphOpt.has_value());
      ASSERT_TRUE(sphOpt->IsSpherical());
      // latitude
      EXPECT_NEAR(sphOpt->Lat()->Degree(), lat.Degree(), 1e-6);
      // longitude
      EXPECT_NEAR(sphOpt->Lon()->Degree(), lon.Degree(), 1e-6);
      // elevation
      EXPECT_NEAR(*sphOpt->Z(), elev, 1e-6);

      // 200 km offset in x (pi/2 heading offset means North). We use
      // SphericalFromLocal, which means that xyz is a linear movement on
      // a plane (not along the curvature of Earth). This will result in
      // a large height offset.
      xyz.SetMetric(2e5, 0, 0);
      sphOpt = sc.SphericalFromLocalPosition(xyz);
      ASSERT_TRUE(sphOpt.has_value());
      ASSERT_TRUE(sphOpt->IsSpherical());
      // increase in latitude about 1.8 degrees
      EXPECT_NEAR(sphOpt->Lat()->Degree(), lat.Degree() + 1.8, 0.008);
      // no change in longitude
      EXPECT_NEAR(*sphOpt->Z(), 3507.024791, 1e-6);

      auto xyz2 = sc.LocalFromSphericalPosition(*sphOpt);
      ASSERT_TRUE(xyz2.has_value());
      EXPECT_EQ(xyz, *xyz2);
    }

    // Check position projection
    {
      // WGS84 coordinate obtained from online mapping software
      // > gdaltransform -s_srs WGS84 -t_srs EPSG:4978
      // > latitude longitude altitude
      // > X Y Z
      std::optional<math::CoordinateVector3> tmp;
      auto osrf_s = math::CoordinateVector3::Spherical(
        GZ_DTOR(37.3877349), GZ_DTOR(-122.0651166), 32.0);
      auto osrf_e = math::CoordinateVector3::Metric(
          -2693701.91434394, -4299942.14687992, 3851691.0393571);
      auto goog_s = math::CoordinateVector3::Spherical(
        GZ_DTOR(37.4216719), GZ_DTOR(-122.0821853), 30.0);

      // Local tangent plane coordinates (ENU = GLOBAL) coordinates of
      // Google when OSRF is taken as the origin:
      // > proj +ellps=WGS84  +proj=tmerc
      // +lat_0=37.3877349 +lon_0=-122.0651166 +k=1 +x_0=0 +y_0=0
      // > -122.0821853 37.4216719 (LON,LAT)
      // > -1510.88 3766.64 (EAST,NORTH)
      auto vec = math::CoordinateVector3::Metric(-1510.88, 3766.64, -3.29);

      // Set the ORIGIN to be the Open Source Robotics Foundation
      math::SphericalCoordinates sc2(st, *osrf_s.Lat(),
          *osrf_s.Lon(), *osrf_s.Z(), math::Angle::Zero);

      // Check that SPHERICAL -> ECEF works
      tmp = sc2.PositionTransform(osrf_s,
          math::SphericalCoordinates::SPHERICAL,
          math::SphericalCoordinates::ECEF);

      ASSERT_TRUE(tmp.has_value());
      ASSERT_TRUE(tmp->IsMetric());
      EXPECT_NEAR(*tmp->X(), *osrf_e.X(), 8e-2);
      EXPECT_NEAR(*tmp->Y(), *osrf_e.Y(), 8e-2);
      EXPECT_NEAR(*tmp->Z(), *osrf_e.Z(), 1e-2);

      // Check that ECEF -> SPHERICAL works
      tmp = sc2.PositionTransform(*tmp,
          math::SphericalCoordinates::ECEF,
          math::SphericalCoordinates::SPHERICAL);

      ASSERT_TRUE(tmp.has_value());
      ASSERT_TRUE(tmp->IsSpherical());
      EXPECT_NEAR(tmp->Lat()->Radian(), osrf_s.Lat()->Radian(), 1e-2);
      EXPECT_NEAR(tmp->Lon()->Radian(), osrf_s.Lon()->Radian(), 1e-2);
      EXPECT_NEAR(*tmp->Z(), *osrf_s.Z(), 1e-2);

      // Check that SPHERICAL -> LOCAL works
      tmp = sc2.LocalFromSphericalPosition(goog_s);
      ASSERT_TRUE(tmp.has_value());
      ASSERT_TRUE(tmp->IsMetric());
      EXPECT_NEAR(*tmp->X(), *vec.X(), 8e-2);
      EXPECT_NEAR(*tmp->Y(), *vec.Y(), 8e-2);
      EXPECT_NEAR(*tmp->Z(), *vec.Z(), 1e-2);

      // Check that SPHERICAL -> LOCAL -> SPHERICAL works
      tmp = sc2.SphericalFromLocalPosition(*tmp);
      ASSERT_TRUE(tmp.has_value());
      ASSERT_TRUE(tmp->IsSpherical());
      EXPECT_NEAR(tmp->Lat()->Radian(), goog_s.Lat()->Radian(), 8e-2);
      EXPECT_NEAR(tmp->Lon()->Radian(), goog_s.Lon()->Radian(), 8e-2);
      EXPECT_NEAR(*tmp->Z(), *goog_s.Z(), 1e-2);
    }
  }

  // Give no heading offset to confirm ENU frame
  {
    math::Angle lat(0.3), lon(-1.2), heading(0.0);
    double elev = 354.1;
    math::SphericalCoordinates sc(st, lat, lon, elev, heading);

    // Check GlobalFromLocal with no heading offset
    {
      // local frame
      math::CoordinateVector3 xyz;
      // east, north, up
      std::optional<math::CoordinateVector3> enuOpt;
      std::optional<math::CoordinateVector3> xyzOpt;

      xyz.SetMetric(1, 0, 0);
      enuOpt = sc.VelocityTransform(xyz,
        math::SphericalCoordinates::LOCAL,
        math::SphericalCoordinates::GLOBAL);
      ASSERT_TRUE(enuOpt.has_value());
      EXPECT_EQ(xyz, *enuOpt);
      xyzOpt = sc.LocalFromGlobalVelocity(*enuOpt);
      ASSERT_TRUE(xyzOpt.has_value());
      EXPECT_EQ(xyz, *xyzOpt);

      xyz.SetMetric(0, 1, 0);
      enuOpt = sc.VelocityTransform(xyz,
        math::SphericalCoordinates::LOCAL,
        math::SphericalCoordinates::GLOBAL);
      ASSERT_TRUE(enuOpt.has_value());
      EXPECT_EQ(xyz, enuOpt);
      xyzOpt = sc.LocalFromGlobalVelocity(*enuOpt);
      ASSERT_TRUE(xyzOpt.has_value());
      EXPECT_EQ(xyz, *xyzOpt);

      xyz.SetMetric(1, -1, 0);
      enuOpt = sc.VelocityTransform(xyz,
        math::SphericalCoordinates::LOCAL,
        math::SphericalCoordinates::GLOBAL);
      ASSERT_TRUE(enuOpt.has_value());
      EXPECT_EQ(xyz, enuOpt);
      xyzOpt = sc.LocalFromGlobalVelocity(*enuOpt);
      ASSERT_TRUE(xyzOpt.has_value());
      EXPECT_EQ(xyz, *xyzOpt);

      xyz.SetMetric(2243.52334, 556.35, 435.6553);
      enuOpt = sc.VelocityTransform(xyz,
        math::SphericalCoordinates::LOCAL,
        math::SphericalCoordinates::GLOBAL);
      ASSERT_TRUE(enuOpt.has_value());
      EXPECT_EQ(xyz, enuOpt);
      xyzOpt = sc.LocalFromGlobalVelocity(*enuOpt);
      ASSERT_TRUE(xyzOpt.has_value());
      EXPECT_EQ(xyz, *xyzOpt);
    }
  }
}

//////////////////////////////////////////////////
// Test coordinate transformations
// TODO(peci1): Remove this test in Gazebo 9
TEST(SphericalCoordinatesTest, CoordinateTransformsDeprecated)
{
  // Default surface type
  math::SphericalCoordinates::SurfaceType st =
    math::SphericalCoordinates::EARTH_WGS84;

  {
    // Parameters
    math::Angle lat(0.3), lon(-1.2),
      heading(math::Angle::HalfPi);
    double elev = 354.1;
    math::SphericalCoordinates sc(st, lat, lon, elev, heading);

    // Check GlobalFromLocal with heading offset of 90 degrees
    // Heading 0:  X == East, Y == North, Z == Up
    // Heading 90: X == North, Y == West , Z == Up
    {
      // local frame
      math::Vector3d xyz;
      // east, north, up
      math::Vector3d enu;

      GZ_UTILS_WARN_IGNORE__DEPRECATED_DECLARATION
      xyz.Set(1, 0, 0);
      enu = sc.GlobalFromLocalVelocity(xyz);
      EXPECT_NEAR(enu.Y()/*North*/, xyz.X(), 1e-6);
      EXPECT_NEAR(enu.X()/*East*/, -xyz.Y(), 1e-6);
      EXPECT_EQ(xyz, sc.LocalFromGlobalVelocity(enu));

      xyz.Set(0, 1, 0);
      enu = sc.GlobalFromLocalVelocity(xyz);
      EXPECT_NEAR(enu.Y(), xyz.X(), 1e-6);
      EXPECT_NEAR(enu.X(), -xyz.Y(), 1e-6);
      EXPECT_EQ(xyz, sc.LocalFromGlobalVelocity(enu));

      xyz.Set(1, -1, 0);
      enu = sc.GlobalFromLocalVelocity(xyz);
      EXPECT_NEAR(enu.Y(), xyz.X(), 1e-6);
      EXPECT_NEAR(enu.X(), -xyz.Y(), 1e-6);
      EXPECT_EQ(xyz, sc.LocalFromGlobalVelocity(enu));

      xyz.Set(2243.52334, 556.35, 435.6553);
      enu = sc.GlobalFromLocalVelocity(xyz);
      EXPECT_NEAR(enu.Y(), xyz.X(), 1e-6);
      EXPECT_NEAR(enu.X(), -xyz.Y(), 1e-6);
      EXPECT_EQ(xyz, sc.LocalFromGlobalVelocity(enu));
      GZ_UTILS_WARN_RESUME__DEPRECATED_DECLARATION
    }

    // Check SphericalFromLocal
    {
      // local frame
      math::Vector3d xyz;
      // spherical coordinates
      math::Vector3d sph;

      GZ_UTILS_WARN_IGNORE__DEPRECATED_DECLARATION
      // No offset
      xyz.Set(0, 0, 0);
      sph = sc.SphericalFromLocalPosition(xyz);
      // latitude
      EXPECT_NEAR(sph.X(), lat.Degree(), 1e-6);
      // longitude
      EXPECT_NEAR(sph.Y(), lon.Degree(), 1e-6);
      // elevation
      EXPECT_NEAR(sph.Z(), elev, 1e-6);

      // 200 km offset in x (pi/2 heading offset means North). We use
      // SphericalFromLocal, which means that xyz is a linear movement on
      // a plane (not along the curvature of Earth). This will result in
      // a large height offset.
      xyz.Set(2e5, 0, 0);
      sph = sc.SphericalFromLocalPosition(xyz);
      // increase in latitude about 1.8 degrees
      EXPECT_NEAR(sph.X(), lat.Degree() + 1.8, 0.008);
      // no change in longitude
      EXPECT_NEAR(sph.Z(), 3507.024791, 1e-6);

      math::Vector3d xyz2 = sc.LocalFromSphericalPosition(sph);
      EXPECT_EQ(xyz, xyz2);
      GZ_UTILS_WARN_RESUME__DEPRECATED_DECLARATION
    }

    // Check position projection
    {
      // WGS84 coordinate obtained from online mapping software
      // > gdaltransform -s_srs WGS84 -t_srs EPSG:4978
      // > latitude longitude altitude
      // > X Y Z
      math::Vector3d tmp;
      math::Vector3d osrf_s(37.3877349, -122.0651166, 32.0);
      math::Vector3d osrf_e(
          -2693701.91434394, -4299942.14687992, 3851691.0393571);
      math::Vector3d goog_s(37.4216719, -122.0821853, 30.0);

      // Local tangent plane coordinates (ENU = GLOBAL) coordinates of
      // Google when OSRF is taken as the origin:
      // > proj +ellps=WGS84  +proj=tmerc
      // +lat_0=37.3877349 +lon_0=-122.0651166 +k=1 +x_0=0 +y_0=0
      // > -122.0821853 37.4216719 (LON,LAT)
      // > -1510.88 3766.64 (EAST,NORTH)
      math::Vector3d vec(-1510.88, 3766.64, -3.29);

      // Convert degrees to radians
      osrf_s.X() *= 0.0174532925;
      osrf_s.Y() *= 0.0174532925;

      // Set the ORIGIN to be the Open Source Robotics Foundation
      math::SphericalCoordinates sc2(st, math::Angle(osrf_s.X()),
          math::Angle(osrf_s.Y()), osrf_s.Z(),
          math::Angle::Zero);

      GZ_UTILS_WARN_IGNORE__DEPRECATED_DECLARATION
      // Check that SPHERICAL -> ECEF works
      tmp = sc2.PositionTransform(osrf_s,
          math::SphericalCoordinates::SPHERICAL,
          math::SphericalCoordinates::ECEF);

      EXPECT_NEAR(tmp.X(), osrf_e.X(), 8e-2);
      EXPECT_NEAR(tmp.Y(), osrf_e.Y(), 8e-2);
      EXPECT_NEAR(tmp.Z(), osrf_e.Z(), 1e-2);

      // Check that ECEF -> SPHERICAL works
      tmp = sc2.PositionTransform(tmp,
          math::SphericalCoordinates::ECEF,
          math::SphericalCoordinates::SPHERICAL);

      EXPECT_NEAR(tmp.X(), osrf_s.X(), 1e-2);
      EXPECT_NEAR(tmp.Y(), osrf_s.Y(), 1e-2);
      EXPECT_NEAR(tmp.Z(), osrf_s.Z(), 1e-2);

      // Check that SPHERICAL -> LOCAL works
      tmp = sc2.LocalFromSphericalPosition(goog_s);
      EXPECT_NEAR(tmp.X(), vec.X(), 8e-2);
      EXPECT_NEAR(tmp.Y(), vec.Y(), 8e-2);
      EXPECT_NEAR(tmp.Z(), vec.Z(), 1e-2);

      // Check that SPHERICAL -> LOCAL -> SPHERICAL works
      tmp = sc2.SphericalFromLocalPosition(tmp);
      EXPECT_NEAR(tmp.X(), goog_s.X(), 8e-2);
      EXPECT_NEAR(tmp.Y(), goog_s.Y(), 8e-2);
      EXPECT_NEAR(tmp.Z(), goog_s.Z(), 1e-2);
      GZ_UTILS_WARN_RESUME__DEPRECATED_DECLARATION
    }
  }

  // Give no heading offset to confirm ENU frame
  {
    math::Angle lat(0.3), lon(-1.2), heading(0.0);
    double elev = 354.1;
    math::SphericalCoordinates sc(st, lat, lon, elev, heading);

    // Check GlobalFromLocal with no heading offset
    {
      // local frame
      math::Vector3d xyz;
      // east, north, up
      math::Vector3d enu;
      math::Vector3d wsu;

      GZ_UTILS_WARN_IGNORE__DEPRECATED_DECLARATION
      xyz.Set(1, 0, 0);
      enu = sc.VelocityTransform(xyz,
        math::SphericalCoordinates::LOCAL2,
        math::SphericalCoordinates::GLOBAL);
      EXPECT_EQ(xyz, enu);
      EXPECT_EQ(xyz, sc.LocalFromGlobalVelocity(enu));

      xyz.Set(0, 1, 0);
      enu = sc.VelocityTransform(xyz,
        math::SphericalCoordinates::LOCAL2,
        math::SphericalCoordinates::GLOBAL);
      EXPECT_EQ(xyz, enu);
      EXPECT_EQ(xyz, sc.LocalFromGlobalVelocity(enu));

      xyz.Set(1, -1, 0);
      enu = sc.VelocityTransform(xyz,
        math::SphericalCoordinates::LOCAL2,
        math::SphericalCoordinates::GLOBAL);
      EXPECT_EQ(xyz, enu);
      EXPECT_EQ(xyz, sc.LocalFromGlobalVelocity(enu));

      xyz.Set(2243.52334, 556.35, 435.6553);
      enu = sc.VelocityTransform(xyz,
        math::SphericalCoordinates::LOCAL2,
        math::SphericalCoordinates::GLOBAL);
      EXPECT_EQ(xyz, enu);
      EXPECT_EQ(xyz, sc.LocalFromGlobalVelocity(enu));

      // This is the incorrect and deprecated behavior of LOCAL frame
      xyz.Set(1, 0, 0);
      wsu = math::Vector3d(-xyz.X(), -xyz.Y(), xyz.Z());
      enu = sc.VelocityTransform(xyz,
        math::SphericalCoordinates::LOCAL,
        math::SphericalCoordinates::GLOBAL);
      EXPECT_EQ(wsu, enu);
      EXPECT_EQ(wsu, sc.LocalFromGlobalVelocity(enu));

      xyz.Set(0, 1, 0);
      wsu = math::Vector3d(-xyz.X(), -xyz.Y(), xyz.Z());
      enu = sc.VelocityTransform(xyz,
        math::SphericalCoordinates::LOCAL,
        math::SphericalCoordinates::GLOBAL);
      EXPECT_EQ(wsu, enu);
      EXPECT_EQ(wsu, sc.LocalFromGlobalVelocity(enu));

      xyz.Set(1, -1, 0);
      wsu = math::Vector3d(-xyz.X(), -xyz.Y(), xyz.Z());
      enu = sc.VelocityTransform(xyz,
        math::SphericalCoordinates::LOCAL,
        math::SphericalCoordinates::GLOBAL);
      EXPECT_EQ(wsu, enu);
      EXPECT_EQ(wsu, sc.LocalFromGlobalVelocity(enu));

      xyz.Set(2243.52334, 556.35, 435.6553);
      wsu = math::Vector3d(-xyz.X(), -xyz.Y(), xyz.Z());
      enu = sc.VelocityTransform(xyz,
        math::SphericalCoordinates::LOCAL,
        math::SphericalCoordinates::GLOBAL);
      EXPECT_EQ(wsu, enu);
      EXPECT_EQ(wsu, sc.LocalFromGlobalVelocity(enu));
      GZ_UTILS_WARN_RESUME__DEPRECATED_DECLARATION
    }
  }
}

//////////////////////////////////////////////////
// Test distance
TEST(SphericalCoordinatesTest, Distance)
{
  math::Angle latA, longA, latB, longB;
  latA.SetDegree(46.250944);
  longA.SetDegree(-122.249972);
  latB.SetDegree(46.124953);
  longB.SetDegree(-122.251683);

  // Calculating distance using the static method.
  double d1 = math::SphericalCoordinates::DistanceWGS84(
      latA, longA, latB, longB);
  EXPECT_NEAR(14002, d1, 20);

  // Using the non static method. The default surface type is EARTH_WGS84.
  auto earthSC = math::SphericalCoordinates();
  double d2 = earthSC.DistanceBetweenPoints(latA, longA, latB, longB);
  EXPECT_NEAR(d1, d2, 0.1);

  earthSC = math::SphericalCoordinates(
      math::SphericalCoordinates::SurfaceType::EARTH_WGS84);
  double d3 = earthSC.DistanceBetweenPoints(latA, longA, latB, longB);
  EXPECT_NEAR(d2, d3, 0.1);

  // Setting the surface type as Moon.
  auto moonSC = math::SphericalCoordinates(
      math::SphericalCoordinates::SurfaceType::MOON_SCS);
  double d4 = moonSC.DistanceBetweenPoints(latA, longA, latB, longB);
  EXPECT_NEAR(3820, d4, 5);

  // Using a custom surface.
  // For custom surfaces, the surface properties need to be set.
  // This one will throw an error.
  auto invalidCustomSC = math::SphericalCoordinates(
      math::SphericalCoordinates::CUSTOM_SURFACE);
  // This one should be accepted.
  auto customSC = math::SphericalCoordinates(
      math::SphericalCoordinates::SurfaceType::CUSTOM_SURFACE,
      6378137.0,
      6356752.314245);

  EXPECT_NEAR(customSC.DistanceBetweenPoints(latA, longA, latB, longB),
      d1, 0.1);
}

//////////////////////////////////////////////////
TEST(SphericalCoordinatesTest, BadSetSurface)
{
  math::SphericalCoordinates sc;
  sc.SetSurface(static_cast<math::SphericalCoordinates::SurfaceType>(3),
      10, 10);
  sc.SetSurface(static_cast<math::SphericalCoordinates::SurfaceType>(3));
  EXPECT_EQ(sc.Surface(), 3);
}

//////////////////////////////////////////////////
TEST(SphericalCoordinatesTest, Transform)
{
  const math::SphericalCoordinates sc;
  const auto vel = math::CoordinateVector3::Metric(1, 2, -4);
  auto result = sc.VelocityTransform(vel,
      math::SphericalCoordinates::ECEF,
      math::SphericalCoordinates::ECEF);
  ASSERT_TRUE(result.has_value());
  ASSERT_TRUE(result->IsMetric());

  EXPECT_EQ(*result, vel);

  const auto pos = math::CoordinateVector3::Metric(-1510.88, 2, -4);
  result = sc.PositionTransform(pos,
      math::SphericalCoordinates::ECEF,
      math::SphericalCoordinates::GLOBAL);
  ASSERT_TRUE(result.has_value());
  ASSERT_TRUE(result->IsMetric());

  EXPECT_NEAR(*result->X(), 2, 1e-6);
  EXPECT_NEAR(*result->Y(), -4, 1e-6);
  EXPECT_NEAR(*result->Z(), -6379647.8799999999, 1e-6);

  std::cout << "NEW POS[" << *result << "]\n";
}

//////////////////////////////////////////////////
// TODO(peci1): Remove this test in Gazebo 9
TEST(SphericalCoordinatesTest, TransformDeprecated)
{
  GZ_UTILS_WARN_IGNORE__DEPRECATED_DECLARATION
  const math::SphericalCoordinates sc;
  constexpr math::Vector3d vel(1, 2, -4);
  math::Vector3d result = sc.VelocityTransform(vel,
      math::SphericalCoordinates::ECEF,
      math::SphericalCoordinates::ECEF);

  EXPECT_EQ(result, vel);

  constexpr math::Vector3d pos(-1510.88, 2, -4);
  result = sc.PositionTransform(pos,
      math::SphericalCoordinates::ECEF,
      math::SphericalCoordinates::GLOBAL);
  GZ_UTILS_WARN_RESUME__DEPRECATED_DECLARATION

  EXPECT_NEAR(result.X(), 2, 1e-6);
  EXPECT_NEAR(result.Y(), -4, 1e-6);
  EXPECT_NEAR(result.Z(), -6379647.8799999999, 1e-6);

  std::cout << "NEW POS[" << result << "]\n";
}

//////////////////////////////////////////////////
TEST(SphericalCoordinatesTest, BadCoordinateType)
{
  math::SphericalCoordinates sc;
  const auto pos = math::CoordinateVector3::Metric(1, 2, -4);
  auto result = sc.PositionTransform(pos,
      static_cast<math::SphericalCoordinates::CoordinateType>(7),
      static_cast<math::SphericalCoordinates::CoordinateType>(6));

  EXPECT_FALSE(result.has_value());

  result = sc.PositionTransform(pos,
      static_cast<math::SphericalCoordinates::CoordinateType>(4),
      static_cast<math::SphericalCoordinates::CoordinateType>(6));

  EXPECT_FALSE(result.has_value());

  result = sc.VelocityTransform(pos,
      math::SphericalCoordinates::SPHERICAL,
      math::SphericalCoordinates::ECEF);
  EXPECT_FALSE(result.has_value());

  result = sc.VelocityTransform(pos,
      math::SphericalCoordinates::ECEF,
      math::SphericalCoordinates::SPHERICAL);
  EXPECT_FALSE(result.has_value());

  result = sc.VelocityTransform(pos,
      static_cast<math::SphericalCoordinates::CoordinateType>(7),
      math::SphericalCoordinates::ECEF);
  EXPECT_FALSE(result.has_value());

  result = sc.VelocityTransform(pos,
      math::SphericalCoordinates::ECEF,
      static_cast<math::SphericalCoordinates::CoordinateType>(7));
  EXPECT_FALSE(result.has_value());
}

//////////////////////////////////////////////////
// TODO(peci1): Remove this test in Gazebo 9
TEST(SphericalCoordinatesTest, BadCoordinateTypeDeprecated)
{
  GZ_UTILS_WARN_IGNORE__DEPRECATED_DECLARATION
  math::SphericalCoordinates sc;
  math::Vector3d pos(1, 2, -4);
  math::Vector3d result = sc.PositionTransform(pos,
      static_cast<math::SphericalCoordinates::CoordinateType>(7),
      static_cast<math::SphericalCoordinates::CoordinateType>(6));

  EXPECT_EQ(result, pos);

  result = sc.PositionTransform(pos,
      static_cast<math::SphericalCoordinates::CoordinateType>(4),
      static_cast<math::SphericalCoordinates::CoordinateType>(6));

  EXPECT_EQ(result, pos);

  result = sc.VelocityTransform(pos,
      math::SphericalCoordinates::SPHERICAL,
      math::SphericalCoordinates::ECEF);
  EXPECT_EQ(result, pos);

  result = sc.VelocityTransform(pos,
      math::SphericalCoordinates::ECEF,
      math::SphericalCoordinates::SPHERICAL);
  EXPECT_EQ(result, pos);

  result = sc.VelocityTransform(pos,
      static_cast<math::SphericalCoordinates::CoordinateType>(7),
      math::SphericalCoordinates::ECEF);
  EXPECT_EQ(result, pos);

  result = sc.VelocityTransform(pos,
      math::SphericalCoordinates::ECEF,
      static_cast<math::SphericalCoordinates::CoordinateType>(7));
  EXPECT_EQ(result, pos);
  GZ_UTILS_WARN_RESUME__DEPRECATED_DECLARATION
}

//////////////////////////////////////////////////
// Test [in]equality operators.
TEST(SphericalCoordinatesTest, EqualityOps)
{
  // Default surface type
  math::SphericalCoordinates::SurfaceType st =
    math::SphericalCoordinates::EARTH_WGS84;
  math::Angle lat(0.3), lon(-1.2), heading(0.5);
  double elev = 354.1;
  math::SphericalCoordinates sc1(st, lat, lon, elev, heading);

  math::SphericalCoordinates sc2(st, lat, lon, elev, heading);
  EXPECT_TRUE(sc1 == sc2);
  EXPECT_FALSE(sc1 != sc2);
  math::SphericalCoordinates sc3(st, math::Angle::Zero, lon, elev,
    heading);
  EXPECT_FALSE(sc1 == sc3);
  EXPECT_TRUE(sc1 != sc3);
  math::SphericalCoordinates sc4(st, lat, math::Angle::Zero, elev,
    heading);
  EXPECT_FALSE(sc1 == sc4);
  EXPECT_TRUE(sc1 != sc4);
  math::SphericalCoordinates sc5(st, lat, lon, elev + 1, heading);
  EXPECT_FALSE(sc1 == sc5);
  EXPECT_TRUE(sc1 != sc5);
  math::SphericalCoordinates sc6(st, lat, lon, elev,
    math::Angle::Zero);
  EXPECT_FALSE(sc1 == sc6);
  EXPECT_TRUE(sc1 != sc6);
}

//////////////////////////////////////////////////
// Test assignment operator.
TEST(SphericalCoordinatesTest, AssignmentOp)
{
  // Default surface type
  math::SphericalCoordinates::SurfaceType st =
    math::SphericalCoordinates::EARTH_WGS84;
  math::Angle lat(0.3), lon(-1.2), heading(0.5);
  double elev = 354.1;
  math::SphericalCoordinates sc1(st, lat, lon, elev, heading);

  math::SphericalCoordinates sc2 = sc1;
  EXPECT_EQ(sc1, sc2);
}

//////////////////////////////////////////////////
TEST(SphericalCoordinatesTest, NoHeading)
{
  // Default heading
  auto st = math::SphericalCoordinates::EARTH_WGS84;
  math::Angle lat(GZ_DTOR(-22.9));
  math::Angle lon(GZ_DTOR(-43.2));
  math::Angle heading(0.0);
  double elev = 0;
  math::SphericalCoordinates sc(st, lat, lon, elev, heading);

  // Origin matches input
  auto latLonAlt = sc.SphericalFromLocalPosition(
    math::CoordinateVector3::Metric(0, 0, 0));
  ASSERT_TRUE(latLonAlt.has_value());
  ASSERT_TRUE(latLonAlt->IsSpherical());
  EXPECT_DOUBLE_EQ(lat.Degree(), latLonAlt->Lat()->Degree());
  EXPECT_DOUBLE_EQ(lon.Degree(), latLonAlt->Lon()->Degree());
  EXPECT_NEAR(elev, *latLonAlt->Z(), 1e-6);

  auto xyzOrigin = sc.LocalFromSphericalPosition(*latLonAlt);
  ASSERT_TRUE(xyzOrigin.has_value());
  ASSERT_TRUE(xyzOrigin->IsMetric());
  EXPECT_EQ(math::Vector3d::Zero, xyzOrigin->AsMetricVector());

  // Check how different lat/lon affect the local position

  // Increase latitude == go North == go +Y
  {
    auto xyz = sc.LocalFromSphericalPosition(math::CoordinateVector3::Spherical(
        lat + GZ_DTOR(1.0), lon, elev));
    ASSERT_TRUE(xyz.has_value());
    ASSERT_TRUE(xyz->IsMetric());
    EXPECT_NEAR(*xyzOrigin->X(), *xyz->X(), 1e-6);
    EXPECT_LT(*xyzOrigin->Y(), *xyz->Y());
  }

  // Decrease latitude == go South == go -Y
  {
    auto xyz = sc.LocalFromSphericalPosition(math::CoordinateVector3::Spherical(
        lat - GZ_DTOR(1.0), lon, elev));
    ASSERT_TRUE(xyz.has_value());
    ASSERT_TRUE(xyz->IsMetric());
    EXPECT_NEAR(*xyzOrigin->X(), *xyz->X(), 1e-6);
    EXPECT_GT(*xyzOrigin->Y(), *xyz->Y());
  }

  // Increase longitude == go East == go +X
  // Also move a bit -Y because this is the Southern Hemisphere
  {
    auto xyz = sc.LocalFromSphericalPosition(math::CoordinateVector3::Spherical(
        lat, lon + GZ_DTOR(1.0), elev));
    ASSERT_TRUE(xyz.has_value());
    ASSERT_TRUE(xyz->IsMetric());
    EXPECT_LT(*xyzOrigin->X(), *xyz->X());
    EXPECT_GT(*xyzOrigin->Y(), *xyz->Y());
  }

  // Decrease longitude == go West == go -X
  // Also move a bit -Y because this is the Southern Hemisphere
  {
    auto xyz = sc.LocalFromSphericalPosition(math::CoordinateVector3::Spherical(
        lat, lon - GZ_DTOR(1.0), elev));
    ASSERT_TRUE(xyz.has_value());
    ASSERT_TRUE(xyz->IsMetric());
    EXPECT_GT(*xyzOrigin->X(), *xyz->X());
    EXPECT_GT(*xyzOrigin->Y(), *xyz->Y());
  }

  // Increase altitude
  {
    auto xyz = sc.LocalFromSphericalPosition(math::CoordinateVector3::Spherical(
        lat, lon, elev + 10.0));
    ASSERT_TRUE(xyz.has_value());
    ASSERT_TRUE(xyz->IsMetric());
    EXPECT_NEAR(*xyzOrigin->X(), *xyz->X(), 1e-6);
    EXPECT_NEAR(*xyzOrigin->Y(), *xyz->Y(), 1e-6);
    EXPECT_NEAR(*xyzOrigin->Z() + 10.0, *xyz->Z(), 1e-6);
  }

  // Decrease altitude
  {
    auto xyz = sc.LocalFromSphericalPosition(math::CoordinateVector3::Spherical(
        lat, lon, elev - 10.0));
    ASSERT_TRUE(xyz.has_value());
    ASSERT_TRUE(xyz->IsMetric());
    EXPECT_NEAR(*xyzOrigin->X(), *xyz->X(), 1e-6);
    EXPECT_NEAR(*xyzOrigin->Y(), *xyz->Y(), 1e-6);
    EXPECT_NEAR(*xyzOrigin->Z() - 10.0, *xyz->Z(), 1e-6);
  }

  // Check how global and local velocities are connected

  // Velocity in
  // +X (East), +Y (North), -X (West), -Y (South), +Z (up), -Z (down)
  for (auto global : {
      math::Vector3d::UnitX,
      math::Vector3d::UnitY,
      math::Vector3d::UnitZ,
      -math::Vector3d::UnitX,
      -math::Vector3d::UnitY,
      -math::Vector3d::UnitZ})
  {
    auto localOpt = sc.LocalFromGlobalVelocity(
      math::CoordinateVector3::Metric(global));
    ASSERT_TRUE(localOpt.has_value());
    ASSERT_TRUE(localOpt->IsMetric());
    EXPECT_EQ(global, localOpt->AsMetricVector());

    auto globalOpt = sc.GlobalFromLocalVelocity(*localOpt);
    ASSERT_TRUE(globalOpt.has_value());
    ASSERT_TRUE(globalOpt->IsMetric());
    EXPECT_EQ(*globalOpt, *localOpt);

    // Directly call VelocityTransform
    globalOpt = sc.VelocityTransform(*localOpt,
        math::SphericalCoordinates::LOCAL,
        math::SphericalCoordinates::GLOBAL);
    ASSERT_TRUE(globalOpt.has_value());
    ASSERT_TRUE(globalOpt->IsMetric());
    EXPECT_EQ(*globalOpt, *localOpt);
  }
}

//////////////////////////////////////////////////
// TODO(peci1): Remove this test in Gazebo 9
TEST(SphericalCoordinatesTest, NoHeadingDeprecated)
{
  // Default heading
  auto st = math::SphericalCoordinates::EARTH_WGS84;
  math::Angle lat(GZ_DTOR(-22.9));
  math::Angle lon(GZ_DTOR(-43.2));
  math::Angle heading(0.0);
  double elev = 0;
  math::SphericalCoordinates sc(st, lat, lon, elev, heading);

  GZ_UTILS_WARN_IGNORE__DEPRECATED_DECLARATION

  // Origin matches input
  auto latLonAlt = sc.SphericalFromLocalPosition({0, 0, 0});
  EXPECT_DOUBLE_EQ(lat.Degree(), latLonAlt.X());
  EXPECT_DOUBLE_EQ(lon.Degree(), latLonAlt.Y());
  EXPECT_NEAR(elev, latLonAlt.Z(), 1e-6);

  auto xyzOrigin = sc.LocalFromSphericalPosition(latLonAlt);
  EXPECT_EQ(math::Vector3d::Zero, xyzOrigin);

  // Check how different lat/lon affect the local position

  // Increase latitude == go North == go +Y
  {
    auto xyz = sc.LocalFromSphericalPosition(
        {lat.Degree() + 1.0, lon.Degree(), elev});
    EXPECT_NEAR(xyzOrigin.X(), xyz.X(), 1e-6);
    EXPECT_LT(xyzOrigin.Y(), xyz.Y());
  }

  // Decrease latitude == go South == go -Y
  {
    auto xyz = sc.LocalFromSphericalPosition(
        {lat.Degree() - 1.0, lon.Degree(), elev});
    EXPECT_NEAR(xyzOrigin.X(), xyz.X(), 1e-6);
    EXPECT_GT(xyzOrigin.Y(), xyz.Y());
  }

  // Increase longitude == go East == go +X
  // Also move a bit -Y because this is the Southern Hemisphere
  {
    auto xyz = sc.LocalFromSphericalPosition(
        {lat.Degree(), lon.Degree() + 1.0, elev});
    EXPECT_LT(xyzOrigin.X(), xyz.X());
    EXPECT_GT(xyzOrigin.Y(), xyz.Y());
  }

  // Decrease longitude == go West == go -X
  // Also move a bit -Y because this is the Southern Hemisphere
  {
    auto xyz = sc.LocalFromSphericalPosition(
        {lat.Degree(), lon.Degree() - 1.0, elev});
    EXPECT_GT(xyzOrigin.X(), xyz.X());
    EXPECT_GT(xyzOrigin.Y(), xyz.Y());
  }

  // Increase altitude
  {
    auto xyz = sc.LocalFromSphericalPosition(
        {lat.Degree(), lon.Degree(), elev + 10.0});
    EXPECT_NEAR(xyzOrigin.X(), xyz.X(), 1e-6);
    EXPECT_NEAR(xyzOrigin.Y(), xyz.Y(), 1e-6);
    EXPECT_NEAR(xyzOrigin.Z() + 10.0, xyz.Z(), 1e-6);
  }

  // Decrease altitude
  {
    auto xyz = sc.LocalFromSphericalPosition(
        {lat.Degree(), lon.Degree(), elev - 10.0});
    EXPECT_NEAR(xyzOrigin.X(), xyz.X(), 1e-6);
    EXPECT_NEAR(xyzOrigin.Y(), xyz.Y(), 1e-6);
    EXPECT_NEAR(xyzOrigin.Z() - 10.0, xyz.Z(), 1e-6);
  }

  // Check how global and local velocities are connected

  math::Vector3d wsu;

  // Velocity in
  // +X (East), +Y (North), -X (West), -Y (South), +Z (up), -Z (down)
  for (auto global : {
      math::Vector3d::UnitX,
      math::Vector3d::UnitY,
      math::Vector3d::UnitZ,
      -math::Vector3d::UnitX,
      -math::Vector3d::UnitY,
      -math::Vector3d::UnitZ})
  {
    auto local = sc.LocalFromGlobalVelocity(global);
    EXPECT_EQ(global, local);

    global = sc.GlobalFromLocalVelocity(local);
    wsu = {-global.X(), -global.Y(), global.Z()};
    EXPECT_EQ(wsu, local);

    // Directly call VelocityTransform
    global = sc.VelocityTransform(local,
        math::SphericalCoordinates::LOCAL,
        math::SphericalCoordinates::GLOBAL);
    wsu = {-global.X(), -global.Y(), global.Z()};
    EXPECT_EQ(wsu, local);
  }

  GZ_UTILS_WARN_RESUME__DEPRECATED_DECLARATION
}

//////////////////////////////////////////////////
TEST(SphericalCoordinatesTest, WithHeading)
{
  // Heading 90 deg: X == North, Y == West , Z == Up
  auto st = math::SphericalCoordinates::EARTH_WGS84;
  math::Angle lat(GZ_DTOR(-22.9));
  math::Angle lon(GZ_DTOR(-43.2));
  math::Angle heading(GZ_DTOR(90.0));
  double elev = 0;
  math::SphericalCoordinates sc(st, lat, lon, elev, heading);

  // Origin matches input
  auto latLonAlt = sc.SphericalFromLocalPosition(
    math::CoordinateVector3::Metric(0, 0, 0));
  ASSERT_TRUE(latLonAlt.has_value());
  ASSERT_TRUE(latLonAlt->IsSpherical());
  EXPECT_DOUBLE_EQ(lat.Degree(), latLonAlt->Lat()->Degree());
  EXPECT_DOUBLE_EQ(lon.Degree(), latLonAlt->Lon()->Degree());
  EXPECT_NEAR(elev, *latLonAlt->Z(), 1e-6);

  auto xyzOrigin = sc.LocalFromSphericalPosition(*latLonAlt);
  ASSERT_TRUE(xyzOrigin.has_value());
  ASSERT_TRUE(xyzOrigin->IsMetric());
  EXPECT_EQ(math::Vector3d::Zero, xyzOrigin->AsMetricVector());

  // Check how different lat/lon affect the local position

  // Increase latitude == go North == go +X
  {
    auto xyz = sc.LocalFromSphericalPosition(math::CoordinateVector3::Spherical(
        lat + GZ_DTOR(1.0), lon, elev));
    ASSERT_TRUE(xyz.has_value());
    ASSERT_TRUE(xyz->IsMetric());
    EXPECT_NEAR(*xyzOrigin->Y(), *xyz->Y(), 1e-6);
    EXPECT_LT(*xyzOrigin->X(), *xyz->X());
  }

  // Decrease latitude == go South == go -X
  {
    auto xyz = sc.LocalFromSphericalPosition(math::CoordinateVector3::Spherical(
        lat - GZ_DTOR(1.0), lon, elev));
    ASSERT_TRUE(xyz.has_value());
    ASSERT_TRUE(xyz->IsMetric());
    EXPECT_NEAR(*xyzOrigin->Y(), *xyz->Y(), 1e-6);
    EXPECT_GT(*xyzOrigin->X(), *xyz->X());
  }

  // Increase longitude == go East == go -Y (and a bit -X)
  {
    auto xyz = sc.LocalFromSphericalPosition(math::CoordinateVector3::Spherical(
        lat, lon + GZ_DTOR(1.0), elev));
    EXPECT_GT(*xyzOrigin->Y(), *xyz->Y());
    EXPECT_GT(*xyzOrigin->X(), *xyz->X());
  }

  // Decrease longitude == go West == go +Y (and a bit -X)
  {
    auto xyz = sc.LocalFromSphericalPosition(math::CoordinateVector3::Spherical(
        lat, lon - GZ_DTOR(1.0), elev));
    EXPECT_LT(*xyzOrigin->Y(), *xyz->Y());
    EXPECT_GT(*xyzOrigin->X(), *xyz->X());
  }

  // Check how global and local velocities are connected

  // Global     | Local
  // ---------- | ------
  // +X (East)  | -Y
  // -X (West)  | +Y
  // +Y (North) | +X
  // -Y (South) | -X
  std::vector<std::pair<math::Vector3d, math::Vector3d>> globalLocal =
      {{math::Vector3d::UnitX, -math::Vector3d::UnitY},
      {-math::Vector3d::UnitX, math::Vector3d::UnitY},
      {math::Vector3d::UnitY, math::Vector3d::UnitX},
      {-math::Vector3d::UnitY, -math::Vector3d::UnitX}};
  for (auto [global, local] : globalLocal)
  {
    auto localRes = sc.LocalFromGlobalVelocity(
      math::CoordinateVector3::Metric(global));
    ASSERT_TRUE(localRes.has_value());
    ASSERT_TRUE(localRes->IsMetric());
    EXPECT_EQ(local, localRes->AsMetricVector());

    // Directly call VelocityTransform
    auto globalRes = sc.VelocityTransform(
        math::CoordinateVector3::Metric(local),
        math::SphericalCoordinates::LOCAL,
        math::SphericalCoordinates::GLOBAL);
    ASSERT_TRUE(globalRes.has_value());
    ASSERT_TRUE(globalRes->IsMetric());
    EXPECT_EQ(global, globalRes->AsMetricVector());
  }
}

//////////////////////////////////////////////////
// TODO(peci1): Remove this test in Gazebo 9
TEST(SphericalCoordinatesTest, WithHeadingDeprecated)
{
  // Heading 90 deg: X == North, Y == West , Z == Up
  auto st = math::SphericalCoordinates::EARTH_WGS84;
  math::Angle lat(GZ_DTOR(-22.9));
  math::Angle lon(GZ_DTOR(-43.2));
  math::Angle heading(GZ_DTOR(90.0));
  double elev = 0;
  math::SphericalCoordinates sc(st, lat, lon, elev, heading);

  GZ_UTILS_WARN_IGNORE__DEPRECATED_DECLARATION

  // Origin matches input
  auto latLonAlt = sc.SphericalFromLocalPosition({0, 0, 0});
  EXPECT_DOUBLE_EQ(lat.Degree(), latLonAlt.X());
  EXPECT_DOUBLE_EQ(lon.Degree(), latLonAlt.Y());
  EXPECT_NEAR(elev, latLonAlt.Z(), 1e-6);

  auto xyzOrigin = sc.LocalFromSphericalPosition(latLonAlt);
  EXPECT_EQ(math::Vector3d::Zero, xyzOrigin);

  // Check how different lat/lon affect the local position

  // Increase latitude == go North == go +X
  {
    auto xyz = sc.LocalFromSphericalPosition(
        {lat.Degree() + 1.0, lon.Degree(), elev});
    EXPECT_NEAR(xyzOrigin.Y(), xyz.Y(), 1e-6);
    EXPECT_LT(xyzOrigin.X(), xyz.X());
  }

  // Decrease latitude == go South == go -X
  {
    auto xyz = sc.LocalFromSphericalPosition(
        {lat.Degree() - 1.0, lon.Degree(), elev});
    EXPECT_NEAR(xyzOrigin.Y(), xyz.Y(), 1e-6);
    EXPECT_GT(xyzOrigin.X(), xyz.X());
  }

  // Increase longitude == go East == go -Y (and a bit -X)
  {
    auto xyz = sc.LocalFromSphericalPosition(
        {lat.Degree(), lon.Degree() + 1.0, elev});
    EXPECT_GT(xyzOrigin.Y(), xyz.Y());
    EXPECT_GT(xyzOrigin.X(), xyz.X());
  }

  // Decrease longitude == go West == go +Y (and a bit -X)
  {
    auto xyz = sc.LocalFromSphericalPosition(
        {lat.Degree(), lon.Degree() - 1.0, elev});
    EXPECT_LT(xyzOrigin.Y(), xyz.Y());
    EXPECT_GT(xyzOrigin.X(), xyz.X());
  }

  // Check how global and local velocities are connected

  // Global     | Local
  // ---------- | ------
  // +X (East)  | -Y
  // -X (West)  | +Y
  // +Y (North) | +X
  // -Y (South) | -X
  std::vector<std::pair<math::Vector3d, math::Vector3d>> globalLocal =
      {{math::Vector3d::UnitX, -math::Vector3d::UnitY},
      {-math::Vector3d::UnitX, math::Vector3d::UnitY},
      {math::Vector3d::UnitY, math::Vector3d::UnitX},
      {-math::Vector3d::UnitY, -math::Vector3d::UnitX}};
  for (auto [global, local] : globalLocal)
  {
    auto localRes = sc.LocalFromGlobalVelocity(global);
    EXPECT_EQ(local, localRes);

    // Directly call VelocityTransform
    auto globalRes = sc.VelocityTransform(local,
        math::SphericalCoordinates::LOCAL,
        math::SphericalCoordinates::GLOBAL);
    EXPECT_EQ(global, globalRes);
  }

  GZ_UTILS_WARN_RESUME__DEPRECATED_DECLARATION
}

//////////////////////////////////////////////////
TEST(SphericalCoordinatesTest, Inverse)
{
  auto st = math::SphericalCoordinates::EARTH_WGS84;
  math::Angle lat(0.3), lon(-1.2), heading(0.5);
  double elev = 354.1;
  math::SphericalCoordinates sc(st, lat, lon, elev, heading);

  // GLOBAL <-> LOCAL
  {
    auto in = math::CoordinateVector3::Metric(1, 2, -4);
    auto out = sc.VelocityTransform(in,
        math::SphericalCoordinates::LOCAL,
        math::SphericalCoordinates::GLOBAL);
    ASSERT_TRUE(out.has_value());
    ASSERT_TRUE(out->IsMetric());
    EXPECT_NE(in, *out);

    auto reverse = sc.VelocityTransform(*out,
        math::SphericalCoordinates::GLOBAL,
        math::SphericalCoordinates::LOCAL);
    ASSERT_TRUE(reverse.has_value());
    ASSERT_TRUE(reverse->IsMetric());
    EXPECT_EQ(in, *reverse);
  }

  {
    auto in = math::CoordinateVector3::Metric(1, 2, -4);
    auto out = sc.PositionTransform(in,
        math::SphericalCoordinates::LOCAL,
        math::SphericalCoordinates::GLOBAL);
    ASSERT_TRUE(out.has_value());
    ASSERT_TRUE(out->IsMetric());
    EXPECT_NE(in, *out);

    auto reverse = sc.PositionTransform(*out,
        math::SphericalCoordinates::GLOBAL,
        math::SphericalCoordinates::LOCAL);
    ASSERT_TRUE(reverse.has_value());
    ASSERT_TRUE(reverse->IsMetric());
    EXPECT_EQ(in, *reverse);
  }

  // SPHERICAL <-> LOCAL
  {
    auto in = math::CoordinateVector3::Metric(1, 2, -4);
    auto out = sc.PositionTransform(in,
        math::SphericalCoordinates::LOCAL,
        math::SphericalCoordinates::SPHERICAL);
    ASSERT_TRUE(out.has_value());
    ASSERT_TRUE(out->IsSpherical());
    EXPECT_NE(in, *out);

    auto reverse = sc.PositionTransform(*out,
        math::SphericalCoordinates::SPHERICAL,
        math::SphericalCoordinates::LOCAL);
    ASSERT_TRUE(reverse.has_value());
    ASSERT_TRUE(reverse->IsMetric());
    EXPECT_EQ(in, *reverse);
  }
}

//////////////////////////////////////////////////
// TODO(peci1): Remove this test in Gazebo 9
TEST(SphericalCoordinatesTest, InverseDeprecated)
{
  auto st = math::SphericalCoordinates::EARTH_WGS84;
  math::Angle lat(0.3), lon(-1.2), heading(0.5);
  double elev = 354.1;
  math::SphericalCoordinates sc(st, lat, lon, elev, heading);

  GZ_UTILS_WARN_IGNORE__DEPRECATED_DECLARATION

  // GLOBAL <-> LOCAL2
  {
    math::Vector3d in(1, 2, -4);
    auto out = sc.VelocityTransform(in,
        math::SphericalCoordinates::LOCAL2,
        math::SphericalCoordinates::GLOBAL);
    EXPECT_NE(in, out);
    auto reverse = sc.VelocityTransform(out,
        math::SphericalCoordinates::GLOBAL,
        math::SphericalCoordinates::LOCAL2);
    EXPECT_EQ(in, reverse);
  }

  {
    math::Vector3d in(1, 2, -4);
    auto out = sc.PositionTransform(in,
        math::SphericalCoordinates::LOCAL2,
        math::SphericalCoordinates::GLOBAL);
    EXPECT_NE(in, out);
    auto reverse = sc.PositionTransform(out,
        math::SphericalCoordinates::GLOBAL,
        math::SphericalCoordinates::LOCAL2);
    EXPECT_EQ(in, reverse);
  }

  // SPHERICAL <-> LOCAL2
  {
    math::Vector3d in(1, 2, -4);
    auto out = sc.PositionTransform(in,
        math::SphericalCoordinates::LOCAL2,
        math::SphericalCoordinates::SPHERICAL);
    EXPECT_NE(in, out);
    auto reverse = sc.PositionTransform(out,
        math::SphericalCoordinates::SPHERICAL,
        math::SphericalCoordinates::LOCAL2);
    EXPECT_EQ(in, reverse);
  }

  // GLOBAL <-> LOCAL
  {
    math::Vector3d in(1, 2, -4);
    auto out = sc.VelocityTransform(in,
        math::SphericalCoordinates::LOCAL,
        math::SphericalCoordinates::GLOBAL);
    EXPECT_NE(in, out);
    auto reverse = sc.VelocityTransform(out,
        math::SphericalCoordinates::GLOBAL,
        math::SphericalCoordinates::LOCAL);
    EXPECT_NE(in, reverse);
  }

  {
    math::Vector3d in(1, 2, -4);
    auto out = sc.PositionTransform(in,
        math::SphericalCoordinates::LOCAL,
        math::SphericalCoordinates::GLOBAL);
    EXPECT_NE(in, out);
    auto reverse = sc.PositionTransform(out,
        math::SphericalCoordinates::GLOBAL,
        math::SphericalCoordinates::LOCAL);
    EXPECT_NE(in, reverse);
  }

  // SPHERICAL <-> LOCAL
  {
    math::Vector3d in(1, 2, -4);
    auto out = sc.PositionTransform(in,
        math::SphericalCoordinates::LOCAL,
        math::SphericalCoordinates::SPHERICAL);
    EXPECT_NE(in, out);
    auto reverse = sc.PositionTransform(out,
        math::SphericalCoordinates::SPHERICAL,
        math::SphericalCoordinates::LOCAL);
    EXPECT_NE(in, reverse);
  }

  GZ_UTILS_WARN_RESUME__DEPRECATED_DECLARATION
}
