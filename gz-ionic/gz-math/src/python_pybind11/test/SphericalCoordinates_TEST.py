# Copyright (C) 2021 Open Source Robotics Foundation
#
# Licensed under the Apache License, Version 2.0 (the "License")
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#       http:#www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import math
import unittest
import warnings

from gz.math8 import Angle, CoordinateVector3, SphericalCoordinates, Vector3d


def ignore_deprecation_warnings(test_func):
    def do_test(self, *args, **kwargs):
        with warnings.catch_warnings():
            warnings.simplefilter("ignore", category=DeprecationWarning)
            test_func(self, *args, **kwargs)
    return do_test


class TestSphericalCoordinates(unittest.TestCase):
    def test_constructor(self):
        # Default surface type
        st = SphericalCoordinates.EARTH_WGS84

        # No arguments, default parameters
        sc = SphericalCoordinates()
        self.assertEqual(sc.surface(), st)
        self.assertEqual(sc.latitude_reference(), Angle())
        self.assertEqual(sc.longitude_reference(), Angle())
        self.assertEqual(sc.heading_offset(), Angle())
        self.assertAlmostEqual(sc.elevation_reference(), 0.0, delta=1e-6)

        # SurfaceType argument, default parameters
        sc = SphericalCoordinates(st)
        self.assertEqual(sc.surface(), st)
        self.assertEqual(sc.latitude_reference(), Angle())
        self.assertEqual(sc.longitude_reference(), Angle())
        self.assertEqual(sc.heading_offset(), Angle())
        self.assertAlmostEqual(sc.elevation_reference(), 0.0, delta=1e-6)

        # All arguments
        lat = Angle(0.3)
        lon = Angle(-1.2)
        heading = Angle(0.5)
        elev = 354.1
        sc = SphericalCoordinates(st, lat, lon, elev, heading)
        self.assertEqual(sc.surface(), st)
        self.assertEqual(sc.latitude_reference(), lat)
        self.assertEqual(sc.longitude_reference(), lon)
        self.assertEqual(sc.heading_offset(), heading)
        self.assertAlmostEqual(sc.elevation_reference(), elev, delta=1e-6)

        # Copy constructor
        sc2 = SphericalCoordinates(sc)
        self.assertEqual(sc, sc2)

    def test_convert(self):
        # Default surface type
        st = SphericalCoordinates.EARTH_WGS84

        self.assertEqual(SphericalCoordinates.convert("EARTH_WGS84"), st)

        self.assertEqual(SphericalCoordinates.EARTH_WGS84,
                         SphericalCoordinates.convert("OTHER-COORD"))

        self.assertEqual("EARTH_WGS84", SphericalCoordinates.convert(st))

        # For Moon surface type
        st = SphericalCoordinates.MOON_SCS
        self.assertEqual(SphericalCoordinates.convert("MOON_SCS"), st)
        self.assertEqual("MOON_SCS", SphericalCoordinates.convert(st))

        # For custom surface type
        st = SphericalCoordinates.CUSTOM_SURFACE
        self.assertEqual(SphericalCoordinates.convert("CUSTOM_SURFACE"),
                st)
        self.assertEqual("CUSTOM_SURFACE",
                SphericalCoordinates.convert(st))

    def test_set_functions(self):
        # Default surface type
        st = SphericalCoordinates.EARTH_WGS84

        # Default parameters
        sc = SphericalCoordinates()
        self.assertEqual(sc.surface(), st)
        self.assertEqual(sc.latitude_reference(), Angle())
        self.assertEqual(sc.longitude_reference(), Angle())
        self.assertEqual(sc.heading_offset(), Angle())
        self.assertAlmostEqual(sc.elevation_reference(), 0.0, delta=1e-6)
        self.assertAlmostEqual(sc.surface_radius(), 6371000.0, delta=1e-3)
        self.assertAlmostEqual(sc.surface_axis_equatorial(),
                6378137.0, delta=1e-3)
        self.assertAlmostEqual(sc.surface_axis_polar(),
                6356752.314245, delta=1e-3)
        self.assertAlmostEqual(sc.surface_flattening(),
                1.0/298.257223563, delta=1e-5)

        lat = Angle(0.3)
        lon = Angle(-1.2)
        heading = Angle(0.5)
        elev = 354.1
        sc.set_surface(st)
        sc.set_latitude_reference(lat)
        sc.set_longitude_reference(lon)
        sc.set_heading_offset(heading)
        sc.set_elevation_reference(elev)

        self.assertEqual(sc.surface(), st)
        self.assertEqual(sc.latitude_reference(), lat)
        self.assertEqual(sc.longitude_reference(), lon)
        self.assertEqual(sc.heading_offset(), heading)
        self.assertAlmostEqual(sc.elevation_reference(), elev, delta=1e-6)
        self.assertAlmostEqual(sc.surface_radius(), 6371000.0, delta=1e-3)
        self.assertAlmostEqual(sc.surface_axis_equatorial(),
                6378137.0, delta=1e-3)
        self.assertAlmostEqual(sc.surface_axis_polar(),
                6356752.314245, delta=1e-3)
        self.assertAlmostEqual(sc.surface_flattening(),
                1.0/298.257223563, delta=1e-5)

        # Moon surface type
        st = SphericalCoordinates.MOON_SCS
        sc = SphericalCoordinates(st)
        sc.set_surface(st)
        self.assertAlmostEqual(sc.surface_radius(), 1737400.0,
                delta=1e-3)
        self.assertAlmostEqual(sc.surface_axis_equatorial(),
                1738100.0, delta=1e-3)
        self.assertAlmostEqual(sc.surface_axis_polar(),
                1736000.0, delta=1e-3)
        self.assertAlmostEqual(sc.surface_flattening(),
                0.0012, delta=1e-5)

    def test_invalid_parameters(self):
        # Earth's constants
        g_EarthWGS84AxisEquatorial = 6378137.0;
        g_EarthWGS84AxisPolar = 6356752.314245;
        g_EarthWGS84Flattening = 1.0/298.257223563;
        g_EarthRadius = 6371000.0;

        # Create a custom surface with invalid parameters
        sc_invalid = SphericalCoordinates(
                SphericalCoordinates.CUSTOM_SURFACE,
                -1, -1)

        # These should be rejected and default to Earth's parameters
        self.assertAlmostEqual(sc_invalid.surface_radius(),
                g_EarthRadius, delta=1e-3)
        self.assertAlmostEqual(sc_invalid.surface_axis_equatorial(),
                g_EarthWGS84AxisEquatorial, delta=1e-3)
        self.assertAlmostEqual(sc_invalid.surface_axis_polar(),
                g_EarthWGS84AxisPolar, delta=1e-3)
        self.assertAlmostEqual(sc_invalid.surface_flattening(),
                g_EarthWGS84Flattening, delta=1e-3)

        # Creating a custom surface with valid parameters
        sc_valid = SphericalCoordinates(
                SphericalCoordinates.CUSTOM_SURFACE,
                100, 100)

        # These should be accepted
        self.assertAlmostEqual(sc_valid.surface_radius(),
                100, delta=1e-3)
        self.assertAlmostEqual(sc_valid.surface_axis_equatorial(),
                100, delta=1e-3)
        self.assertAlmostEqual(sc_valid.surface_axis_polar(),
                100, delta=1e-3)
        self.assertAlmostEqual(sc_valid.surface_flattening(),
                0, delta=1e-3)

    def test_coordinate_transforms(self):
        # Default surface type
        st = SphericalCoordinates.EARTH_WGS84

        # Parameters
        lat = Angle(0.3)
        lon = Angle(-1.2)
        heading = Angle(Angle.HALF_PI)
        elev = 354.1
        sc = SphericalCoordinates(st, lat, lon, elev, heading)

        # Check GlobalFromLocal with heading offset of 90 degrees
        # Heading 0:  X == East, Y == North, Z == Up
        # Heading 90: X == North, Y == West , Z == Up
        # local frame
        xyz = CoordinateVector3()
        # east, north, up
        enu = CoordinateVector3()

        xyz.set_metric(1, 0, 0)
        enu = sc.global_from_local_velocity(xyz)
        self.assertIsNotNone(enu)
        self.assertTrue(enu.is_metric())
        self.assertAlmostEqual(enu.y(), xyz.x(), delta=1e-6)
        self.assertAlmostEqual(enu.x(), -xyz.y(), delta=1e-6)
        self.assertEqual(xyz, sc.local_from_global_velocity(enu))

        xyz.set_metric(0, 1, 0)
        enu = sc.global_from_local_velocity(xyz)
        self.assertIsNotNone(enu)
        self.assertTrue(enu.is_metric())
        self.assertAlmostEqual(enu.y(), xyz.x(), delta=1e-6)
        self.assertAlmostEqual(enu.x(), -xyz.y(), delta=1e-6)
        self.assertEqual(xyz, sc.local_from_global_velocity(enu))

        xyz.set_metric(1, -1, 0)
        enu = sc.global_from_local_velocity(xyz)
        self.assertIsNotNone(enu)
        self.assertTrue(enu.is_metric())
        self.assertAlmostEqual(enu.y(), xyz.x(), delta=1e-6)
        self.assertAlmostEqual(enu.x(), -xyz.y(), delta=1e-6)
        self.assertEqual(xyz, sc.local_from_global_velocity(enu))

        xyz.set_metric(2243.52334, 556.35, 435.6553)
        enu = sc.global_from_local_velocity(xyz)
        self.assertIsNotNone(enu)
        self.assertTrue(enu.is_metric())
        self.assertAlmostEqual(enu.y(), xyz.x(), delta=1e-6)
        self.assertAlmostEqual(enu.x(), -xyz.y(), delta=1e-6)
        self.assertEqual(xyz, sc.local_from_global_velocity(enu))

        # Check SphericalFromLocal
        # local frame
        xyz = CoordinateVector3()
        # spherical coordinates
        sph = CoordinateVector3()

        # No offset
        xyz.set_metric(0, 0, 0)
        sph = sc.spherical_from_local_position(xyz)
        self.assertIsNotNone(sph)
        self.assertTrue(sph.is_spherical())
        # latitude
        self.assertAlmostEqual(sph.lat().degree(), lat.degree(), delta=1e-6)
        # longitude
        self.assertAlmostEqual(sph.lon().degree(), lon.degree(), delta=1e-6)
        # elevation
        self.assertAlmostEqual(sph.z(), elev, delta=1e-6)

        # 200 km offset in x (pi/2 heading offset means North). We use
        # SphericalFromLocal, which means that xyz is a linear movement on
        # a plane (not along the curvature of Earth). This will result in
        # a large height offset.
        xyz.set_metric(2e5, 0, 0)
        sph = sc.spherical_from_local_position(xyz)
        self.assertIsNotNone(sph)
        self.assertTrue(sph.is_spherical())
        # increase in latitude about 1.8 degrees
        self.assertAlmostEqual(
            sph.lat().degree(), lat.degree() + 1.8, delta=0.008)
        # no change in longitude
        self.assertAlmostEqual(sph.z(), 3507.024791, delta=1e-6)

        xyz2 = sc.local_from_spherical_position(sph)
        self.assertIsNotNone(xyz2)
        self.assertTrue(xyz2.is_metric())
        self.assertEqual(xyz, xyz2)

        # Check position projection
        # WGS84 coordinate obtained from online mapping software
        # > gdaltransform -s_srs WGS84 -t_srs EPSG:4978
        # > latitude longitude altitude
        # > X Y Z
        tmp = CoordinateVector3()
        osrf_s = CoordinateVector3.spherical(
            Angle(math.radians(37.3877349)),
            Angle(math.radians(-122.0651166)),
            32.0)
        osrf_e = CoordinateVector3.metric(
            -2693701.91434394, -4299942.14687992, 3851691.0393571)
        goog_s = CoordinateVector3.spherical(
            Angle(math.radians(37.4216719)),
            Angle(math.radians(-122.0821853)),
            30.0)

        # Local tangent plane coordinates (ENU = GLOBAL) coordinates of
        # Google when OSRF is taken as the origin:
        # > proj +ellps=WGS84  +proj=tmerc
        # +lat_0=37.3877349 +lon_0=-122.0651166 +k=1 +x_0=0 +y_0=0
        # > -122.0821853 37.4216719 (LON,LAT)
        # > -1510.88 3766.64 (EAST,NORTH)
        vec = CoordinateVector3.metric(-1510.88, 3766.64, -3.29)

        # Set the ORIGIN to be the Open Source Robotics Foundation
        sc2 = SphericalCoordinates(
            st, osrf_s.lat(), osrf_s.lon(), osrf_s.z(), Angle.ZERO)

        # Check that SPHERICAL -> ECEF works
        tmp = sc2.position_transform(osrf_s, SphericalCoordinates.SPHERICAL,
                                     SphericalCoordinates.ECEF)

        self.assertIsNotNone(tmp)
        self.assertTrue(tmp.is_metric())
        self.assertAlmostEqual(tmp.x(), osrf_e.x(), delta=8e-2)
        self.assertAlmostEqual(tmp.y(), osrf_e.y(), delta=8e-2)
        self.assertAlmostEqual(tmp.z(), osrf_e.z(), delta=1e-2)

        # Check that ECEF -> SPHERICAL works
        tmp = sc2.position_transform(tmp, SphericalCoordinates.ECEF,
                                     SphericalCoordinates.SPHERICAL)

        self.assertIsNotNone(tmp)
        self.assertTrue(tmp.is_spherical())
        self.assertAlmostEqual(
            tmp.lat().degree(), osrf_s.lat().degree(), delta=1e-2)
        self.assertAlmostEqual(
            tmp.lon().degree(), osrf_s.lon().degree(), delta=1e-2)
        self.assertAlmostEqual(tmp.z(), osrf_s.z(), delta=1e-2)

        # Check that SPHERICAL -> LOCAL works
        tmp = sc2.local_from_spherical_position(goog_s)
        self.assertIsNotNone(tmp)
        self.assertTrue(tmp.is_metric())
        self.assertAlmostEqual(tmp.x(), vec.x(), delta=8e-2)
        self.assertAlmostEqual(tmp.y(), vec.y(), delta=8e-2)
        self.assertAlmostEqual(tmp.z(), vec.z(), delta=1e-2)

        # Check that SPHERICAL -> LOCAL -> SPHERICAL works
        tmp = sc2.spherical_from_local_position(tmp)
        self.assertIsNotNone(tmp)
        self.assertTrue(tmp.is_spherical())
        self.assertAlmostEqual(
            tmp.lat().degree(), goog_s.lat().degree(), delta=8e-2)
        self.assertAlmostEqual(
            tmp.lon().degree(), goog_s.lon().degree(), delta=8e-2)
        self.assertAlmostEqual(tmp.z(), goog_s.z(), delta=1e-2)

        # Give no heading offset to confirm ENU frame
        lat = Angle(0.3)
        lon = Angle(-1.2)
        heading = Angle(0.0)
        elev = 354.1
        sc = SphericalCoordinates(st, lat, lon, elev, heading)

        # Check GlobalFromLocal with no heading offset
        # local frame
        xyz = CoordinateVector3()
        # east, north, up
        enu = CoordinateVector3()

        xyz.set_metric(1, 0, 0)
        enu = sc.velocity_transform(xyz, SphericalCoordinates.LOCAL, SphericalCoordinates.GLOBAL)
        self.assertIsNotNone(enu)
        self.assertTrue(enu.is_metric())
        self.assertEqual(xyz, enu)
        self.assertEqual(xyz, sc.local_from_global_velocity(enu))

        xyz.set_metric(0, 1, 0)
        enu = sc.velocity_transform(xyz, SphericalCoordinates.LOCAL, SphericalCoordinates.GLOBAL)
        self.assertIsNotNone(enu)
        self.assertTrue(enu.is_metric())
        self.assertEqual(xyz, enu)
        self.assertEqual(xyz, sc.local_from_global_velocity(enu))

        xyz.set_metric(1, -1, 0)
        enu = sc.velocity_transform(xyz, SphericalCoordinates.LOCAL, SphericalCoordinates.GLOBAL)
        self.assertIsNotNone(enu)
        self.assertTrue(enu.is_metric())
        self.assertEqual(xyz, enu)
        self.assertEqual(xyz, sc.local_from_global_velocity(enu))

        xyz.set_metric(2243.52334, 556.35, 435.6553)
        enu = sc.velocity_transform(xyz, SphericalCoordinates.LOCAL2, SphericalCoordinates.GLOBAL)
        self.assertIsNotNone(enu)
        self.assertTrue(enu.is_metric())
        self.assertEqual(xyz, enu)
        self.assertEqual(xyz, sc.local_from_global_velocity(enu))

    # TODO(peci1): Remove this test in Gazebo 9
    @ignore_deprecation_warnings
    def test_coordinate_transforms_deprecated(self):
        # Default surface type
        st = SphericalCoordinates.EARTH_WGS84

        # Parameters
        lat = Angle(0.3)
        lon = Angle(-1.2)
        heading = Angle(Angle.HALF_PI)
        elev = 354.1
        sc = SphericalCoordinates(st, lat, lon, elev, heading)

        # Check GlobalFromLocal with heading offset of 90 degrees
        # Heading 0:  X == East, Y == North, Z == Up
        # Heading 90: X == North, Y == West , Z == Up
        # local frame
        xyz = Vector3d()
        # east, north, up
        enu = Vector3d()

        xyz.set(1, 0, 0)
        enu = sc.global_from_local_velocity(xyz)
        self.assertAlmostEqual(enu.y(), xyz.x(), delta=1e-6)
        self.assertAlmostEqual(enu.x(), -xyz.y(), delta=1e-6)
        self.assertEqual(xyz, sc.local_from_global_velocity(enu))

        xyz.set(0, 1, 0)
        enu = sc.global_from_local_velocity(xyz)
        self.assertAlmostEqual(enu.y(), xyz.x(), delta=1e-6)
        self.assertAlmostEqual(enu.x(), -xyz.y(), delta=1e-6)
        self.assertEqual(xyz, sc.local_from_global_velocity(enu))

        xyz.set(1, -1, 0)
        enu = sc.global_from_local_velocity(xyz)
        self.assertAlmostEqual(enu.y(), xyz.x(), delta=1e-6)
        self.assertAlmostEqual(enu.x(), -xyz.y(), delta=1e-6)
        self.assertEqual(xyz, sc.local_from_global_velocity(enu))

        xyz.set(2243.52334, 556.35, 435.6553)
        enu = sc.global_from_local_velocity(xyz)
        self.assertAlmostEqual(enu.y(), xyz.x(), delta=1e-6)
        self.assertAlmostEqual(enu.x(), -xyz.y(), delta=1e-6)
        self.assertEqual(xyz, sc.local_from_global_velocity(enu))

        # Check SphericalFromLocal
        # local frame
        xyz = Vector3d()
        # spherical coordinates
        sph = Vector3d()

        # No offset
        xyz.set(0, 0, 0)
        sph = sc.spherical_from_local_position(xyz)
        # latitude
        self.assertAlmostEqual(sph.x(), lat.degree(), delta=1e-6)
        # longitude
        self.assertAlmostEqual(sph.y(), lon.degree(), delta=1e-6)
        # elevation
        self.assertAlmostEqual(sph.z(), elev, delta=1e-6)

        # 200 km offset in x (pi/2 heading offset means North). We use
        # SphericalFromLocal, which means that xyz is a linear movement on
        # a plane (not along the curvature of Earth). This will result in
        # a large height offset.
        xyz.set(2e5, 0, 0)
        sph = sc.spherical_from_local_position(xyz)
        # increase in latitude about 1.8 degrees
        self.assertAlmostEqual(sph.x(), lat.degree() + 1.8, delta=0.008)
        # no change in longitude
        self.assertAlmostEqual(sph.z(), 3507.024791, delta=1e-6)

        xyz2 = sc.local_from_spherical_position(sph)
        self.assertEqual(xyz, xyz2)

        # Check position projection
        # WGS84 coordinate obtained from online mapping software
        # > gdaltransform -s_srs WGS84 -t_srs EPSG:4978
        # > latitude longitude altitude
        # > X Y Z
        tmp = Vector3d()
        osrf_s = Vector3d(37.3877349, -122.0651166, 32.0)
        osrf_e = Vector3d(-2693701.91434394, -4299942.14687992, 3851691.0393571)
        goog_s = Vector3d(37.4216719, -122.0821853, 30.0)

        # Local tangent plane coordinates (ENU = GLOBAL) coordinates of
        # Google when OSRF is taken as the origin:
        # > proj +ellps=WGS84  +proj=tmerc
        # +lat_0=37.3877349 +lon_0=-122.0651166 +k=1 +x_0=0 +y_0=0
        # > -122.0821853 37.4216719 (LON,LAT)
        # > -1510.88 3766.64 (EAST,NORTH)
        vec = Vector3d(-1510.88, 3766.64, -3.29)

        # Convert degrees to radians
        osrf_s.x(osrf_s.x() * 0.0174532925)
        osrf_s.y(osrf_s.y() * 0.0174532925)

        # Set the ORIGIN to be the Open Source Robotics Foundation
        sc2 = SphericalCoordinates(st, Angle(osrf_s.x()),
                                   Angle(osrf_s.y()), osrf_s.z(), Angle.ZERO)

        # Check that SPHERICAL -> ECEF works
        tmp = sc2.position_transform(osrf_s, SphericalCoordinates.SPHERICAL,
                                     SphericalCoordinates.ECEF)

        self.assertAlmostEqual(tmp.x(), osrf_e.x(), delta=8e-2)
        self.assertAlmostEqual(tmp.y(), osrf_e.y(), delta=8e-2)
        self.assertAlmostEqual(tmp.z(), osrf_e.z(), delta=1e-2)

        # Check that ECEF -> SPHERICAL works
        tmp = sc2.position_transform(tmp, SphericalCoordinates.ECEF, SphericalCoordinates.SPHERICAL)

        self.assertAlmostEqual(tmp.x(), osrf_s.x(), delta=1e-2)
        self.assertAlmostEqual(tmp.y(), osrf_s.y(), delta=1e-2)
        self.assertAlmostEqual(tmp.z(), osrf_s.z(), delta=1e-2)

        # Check that SPHERICAL -> LOCAL works
        tmp = sc2.local_from_spherical_position(goog_s)
        self.assertAlmostEqual(tmp.x(), vec.x(), delta=8e-2)
        self.assertAlmostEqual(tmp.y(), vec.y(), delta=8e-2)
        self.assertAlmostEqual(tmp.z(), vec.z(), delta=1e-2)

        # Check that SPHERICAL -> LOCAL -> SPHERICAL works
        tmp = sc2.spherical_from_local_position(tmp)
        self.assertAlmostEqual(tmp.x(), goog_s.x(), delta=8e-2)
        self.assertAlmostEqual(tmp.y(), goog_s.y(), delta=8e-2)
        self.assertAlmostEqual(tmp.z(), goog_s.z(), delta=1e-2)

        # Give no heading offset to confirm ENU frame
        lat = Angle(0.3)
        lon = Angle(-1.2)
        heading = Angle(0.0)
        elev = 354.1
        sc = SphericalCoordinates(st, lat, lon, elev, heading)

        # Check GlobalFromLocal with no heading offset
        # local frame
        xyz = Vector3d()
        # east, north, up
        enu = Vector3d()

        xyz.set(1, 0, 0)
        enu = sc.velocity_transform(xyz, SphericalCoordinates.LOCAL2, SphericalCoordinates.GLOBAL)
        self.assertEqual(xyz, enu)
        self.assertEqual(xyz, sc.local_from_global_velocity(enu))

        xyz.set(0, 1, 0)
        enu = sc.velocity_transform(xyz, SphericalCoordinates.LOCAL2, SphericalCoordinates.GLOBAL)
        self.assertEqual(xyz, enu)
        self.assertEqual(xyz, sc.local_from_global_velocity(enu))

        xyz.set(1, -1, 0)
        enu = sc.velocity_transform(xyz, SphericalCoordinates.LOCAL2, SphericalCoordinates.GLOBAL)
        self.assertEqual(xyz, enu)
        self.assertEqual(xyz, sc.local_from_global_velocity(enu))

        xyz.set(2243.52334, 556.35, 435.6553)
        enu = sc.velocity_transform(xyz, SphericalCoordinates.LOCAL2, SphericalCoordinates.GLOBAL)
        self.assertEqual(xyz, enu)
        self.assertEqual(xyz, sc.local_from_global_velocity(enu))

        # This is the incorrect and deprecated behavior of LOCAL frame

        xyz.set(1, 0, 0)
        wsu = Vector3d(-xyz.x(), -xyz.y(), xyz.z())
        enu = sc.velocity_transform(xyz, SphericalCoordinates.LOCAL, SphericalCoordinates.GLOBAL)
        self.assertEqual(wsu, enu)
        self.assertEqual(wsu, sc.local_from_global_velocity(enu))

        xyz.set(0, 1, 0)
        wsu = Vector3d(-xyz.x(), -xyz.y(), xyz.z())
        enu = sc.velocity_transform(xyz, SphericalCoordinates.LOCAL, SphericalCoordinates.GLOBAL)
        self.assertEqual(wsu, enu)
        self.assertEqual(wsu, sc.local_from_global_velocity(enu))

        xyz.set(1, -1, 0)
        wsu = Vector3d(-xyz.x(), -xyz.y(), xyz.z())
        enu = sc.velocity_transform(xyz, SphericalCoordinates.LOCAL, SphericalCoordinates.GLOBAL)
        self.assertEqual(wsu, enu)
        self.assertEqual(wsu, sc.local_from_global_velocity(enu))

        xyz.set(2243.52334, 556.35, 435.6553)
        wsu = Vector3d(-xyz.x(), -xyz.y(), xyz.z())
        enu = sc.velocity_transform(xyz, SphericalCoordinates.LOCAL, SphericalCoordinates.GLOBAL)
        self.assertEqual(wsu, enu)
        self.assertEqual(wsu, sc.local_from_global_velocity(enu))

    def test_distance(self):
        latA = Angle()
        longA = Angle()
        latB = Angle()
        longB = Angle()
        latA.set_degree(46.250944)
        longA.set_degree(-122.249972)
        latB.set_degree(46.124953)
        longB.set_degree(-122.251683)
        d1 = SphericalCoordinates.distance_WGS84(latA, longA, latB, longB)

        self.assertAlmostEqual(14002, d1, delta=20)

        # Using the non static method. The default surface is EARTH_WGS84.
        earth_sc = SphericalCoordinates()
        d2 = earth_sc.distance_between_points(latA, longA, latB, longB)
        self.assertAlmostEqual(d1, d2, delta=0.1)

        earth_sc = SphericalCoordinates(SphericalCoordinates.EARTH_WGS84)
        d3 = earth_sc.distance_between_points(latA, longA, latB, longB)
        self.assertAlmostEqual(d2, d3, delta=0.1)

        # Using the surface type as Moon.
        moon_sc = SphericalCoordinates(SphericalCoordinates.MOON_SCS)
        d4 = moon_sc.distance_between_points(latA, longA, latB, longB)
        self.assertAlmostEqual(3820, d4, delta=5)

        # Using a custom surface
        # For custom surfaces, the surface properties need to be set.
        # This line should throw an error.
        invalid_custom_sc = SphericalCoordinates(
                SphericalCoordinates.CUSTOM_SURFACE)
        # This one should be accepted.
        valid_custom_sc = SphericalCoordinates(
            SphericalCoordinates.CUSTOM_SURFACE,
            6378137.0,
            6356752.314245);

        self.assertAlmostEqual(valid_custom_sc.distance_between_points(latA, longA, latB, longB),
                d1, delta=0.1)

    def test_bad_set_surface(self):
        sc = SphericalCoordinates()
        sc.set_surface(SphericalCoordinates.SurfaceType(3))
        self.assertEqual(sc.surface(), SphericalCoordinates.SurfaceType(3))

    def test_transform(self):
        sc = SphericalCoordinates()
        vel = CoordinateVector3.metric(1, 2, -4)
        result = sc.velocity_transform(
            vel,
            SphericalCoordinates.ECEF,
            SphericalCoordinates.ECEF)

        self.assertIsNotNone(result)
        self.assertTrue(result.is_metric())
        self.assertEqual(result, vel)

        pos = CoordinateVector3.metric(-1510.88, 2, -4)
        result = sc.position_transform(
            pos,
            SphericalCoordinates.ECEF,
            SphericalCoordinates.GLOBAL)

        self.assertIsNotNone(result)
        self.assertTrue(result.is_metric())
        self.assertAlmostEqual(result.x(), 2, delta=1e-6)
        self.assertAlmostEqual(result.y(), -4, delta=1e-6)
        self.assertAlmostEqual(result.z(), -6379647.8799999999, delta=1e-6)

        print('NEW POS[', result.x(), ' ', result.y(), ' ', result.z(), ']\n')

    # TODO(peci1): Remove this test in Gazebo 9
    @ignore_deprecation_warnings
    def test_transform_deprecated(self):
        sc = SphericalCoordinates()
        vel = Vector3d(1, 2, -4)
        result = sc.velocity_transform(
            vel,
            SphericalCoordinates.ECEF,
            SphericalCoordinates.ECEF)

        self.assertEqual(result, vel)

        pos = Vector3d(-1510.88, 2, -4)
        result = sc.position_transform(
            pos,
            SphericalCoordinates.ECEF,
            SphericalCoordinates.GLOBAL)

        self.assertAlmostEqual(result.x(), 2, delta=1e-6)
        self.assertAlmostEqual(result.y(), -4, delta=1e-6)
        self.assertAlmostEqual(result.z(), -6379647.8799999999, delta=1e-6)

        print('NEW POS[', result.x(), ' ', result.y(), ' ', result.z(), ']\n')

    def test_bad_coordinate_type(self):
        sc = SphericalCoordinates()
        pos = CoordinateVector3.metric(1, 2, -4)
        result = sc.position_transform(pos,
                                       SphericalCoordinates.CoordinateType(7),
                                       SphericalCoordinates.CoordinateType(6))

        self.assertIsNone(result)

        result = sc.position_transform(pos,
                                       SphericalCoordinates.CoordinateType(4),
                                       SphericalCoordinates.CoordinateType(6))

        self.assertIsNone(result)

        result = sc.velocity_transform(
            pos,
            SphericalCoordinates.SPHERICAL,
            SphericalCoordinates.ECEF)
        self.assertIsNone(result)

        result = sc.velocity_transform(
            pos,
            SphericalCoordinates.ECEF,
            SphericalCoordinates.SPHERICAL)
        self.assertIsNone(result)

        result = sc.velocity_transform(pos,
                                       SphericalCoordinates.CoordinateType(7),
                                       SphericalCoordinates.ECEF)
        self.assertIsNone(result)

        result = sc.velocity_transform(pos,
                                       SphericalCoordinates.ECEF,
                                       SphericalCoordinates.CoordinateType(7))
        self.assertIsNone(result)

    # TODO(peci1): Remove this test in Gazebo 9
    @ignore_deprecation_warnings
    def test_bad_coordinate_type_deprecated(self):
        sc = SphericalCoordinates()
        pos = Vector3d(1, 2, -4)
        result = sc.position_transform(pos,
                                       SphericalCoordinates.CoordinateType(7),
                                       SphericalCoordinates.CoordinateType(6))

        self.assertEqual(result, pos)

        result = sc.position_transform(pos,
                                       SphericalCoordinates.CoordinateType(4),
                                       SphericalCoordinates.CoordinateType(6))

        self.assertEqual(result, pos)

        result = sc.velocity_transform(
            pos,
            SphericalCoordinates.SPHERICAL,
            SphericalCoordinates.ECEF)
        self.assertEqual(result, pos)

        result = sc.velocity_transform(
            pos,
            SphericalCoordinates.ECEF,
            SphericalCoordinates.SPHERICAL)
        self.assertEqual(result, pos)

        result = sc.velocity_transform(pos,
                                       SphericalCoordinates.CoordinateType(7),
                                       SphericalCoordinates.ECEF)
        self.assertEqual(result, pos)

        result = sc.velocity_transform(pos,
                                       SphericalCoordinates.ECEF,
                                       SphericalCoordinates.CoordinateType(7))
        self.assertEqual(result, pos)

    def test_equality_ops(self):
        # Default surface type
        st = SphericalCoordinates.EARTH_WGS84
        lat = Angle(0.3)
        lon = Angle(-1.2)
        heading = Angle(0.5)
        elev = 354.1
        sc1 = SphericalCoordinates(st, lat, lon, elev, heading)

        sc2 = SphericalCoordinates(st, lat, lon, elev, heading)
        self.assertTrue(sc1 == sc2)
        self.assertFalse(sc1 != sc2)
        sc3 = SphericalCoordinates(st, Angle.ZERO, lon, elev, heading)
        self.assertFalse(sc1 == sc3)
        self.assertTrue(sc1 != sc3)
        sc4 = SphericalCoordinates(st, lat, Angle.ZERO, elev, heading)
        self.assertFalse(sc1 == sc4)
        self.assertTrue(sc1 != sc4)
        sc5 = SphericalCoordinates(st, lat, lon, elev + 1, heading)
        self.assertFalse(sc1 == sc5)
        self.assertTrue(sc1 != sc5)
        sc6 = SphericalCoordinates(st, lat, lon, elev, Angle.ZERO)
        self.assertFalse(sc1 == sc6)
        self.assertTrue(sc1 != sc6)

    def test_assigment_op(self):
        # Default surface type
        st = SphericalCoordinates.EARTH_WGS84
        lat = Angle(0.3)
        lon = Angle(-1.2)
        heading = Angle(0.5)
        elev = 354.1
        sc1 = SphericalCoordinates(st, lat, lon, elev, heading)

        sc2 = sc1
        self.assertEqual(sc1, sc2)

    def test_no_heading(self):
        # Default heading
        st = SphericalCoordinates.EARTH_WGS84
        lat = Angle(-22.9 * math.pi / 180.0)
        lon = Angle(-43.2 * math.pi / 180.0)
        heading = Angle(0.0)
        elev = 0
        sc = SphericalCoordinates(st, lat, lon, elev, heading)

        # Origin matches input
        latLonAlt = sc.spherical_from_local_position(
            CoordinateVector3.metric(0, 0, 0))
        self.assertIsNotNone(latLonAlt)
        self.assertTrue(latLonAlt.is_spherical())
        self.assertEqual(lat.degree(), latLonAlt.lat().degree())
        self.assertEqual(lon.degree(), latLonAlt.lon().degree())
        self.assertEqual(elev, latLonAlt.z())

        xyzOrigin = sc.local_from_spherical_position(latLonAlt)
        self.assertIsNotNone(xyzOrigin)
        self.assertTrue(xyzOrigin.is_metric())
        self.assertEqual(Vector3d.ZERO, xyzOrigin.as_metric_vector())

        # Check how different lat/lon affect the local position

        # Increase latitude == go North == go +Y
        xyz = sc.local_from_spherical_position(CoordinateVector3.spherical(
            lat + Angle(math.radians(1.0)), lon, elev))
        self.assertIsNotNone(xyz)
        self.assertTrue(xyz.is_metric())
        self.assertAlmostEqual(xyzOrigin.x(), xyz.x(), delta=1e-6)
        self.assertLess(xyzOrigin.y(), xyz.y())

        # Decrease latitude == go South == go -Y
        xyz = sc.local_from_spherical_position(CoordinateVector3.spherical(
            lat - Angle(math.radians(1.0)), lon, elev))
        self.assertIsNotNone(xyz)
        self.assertTrue(xyz.is_metric())
        self.assertAlmostEqual(xyzOrigin.x(), xyz.x(), delta=1e-6)
        self.assertGreater(xyzOrigin.y(), xyz.y())

        # Increase longitude == go East == go +X
        # Also move a bit -Y because this is the Southern Hemisphere
        xyz = sc.local_from_spherical_position(CoordinateVector3.spherical(
            lat, lon + Angle(math.radians(1.0)), elev))
        self.assertIsNotNone(xyz)
        self.assertTrue(xyz.is_metric())
        self.assertLess(xyzOrigin.x(), xyz.x())
        self.assertGreater(xyzOrigin.y(), xyz.y())

        # Decrease longitude == go West == go -X
        # Also move a bit -Y because this is the Southern Hemisphere
        xyz = sc.local_from_spherical_position(CoordinateVector3.spherical(
            lat, lon - Angle(math.radians(1.0)), elev))
        self.assertIsNotNone(xyz)
        self.assertTrue(xyz.is_metric())
        self.assertGreater(xyzOrigin.x(), xyz.x())
        self.assertGreater(xyzOrigin.y(), xyz.y())

        # Increase altitude
        xyz = sc.local_from_spherical_position(CoordinateVector3.spherical(
            lat, lon, elev + 10.0))
        self.assertIsNotNone(xyz)
        self.assertTrue(xyz.is_metric())
        self.assertAlmostEqual(xyzOrigin.x(), xyz.x(), delta=1e-6)
        self.assertAlmostEqual(xyzOrigin.y(), xyz.y(), delta=1e-6)
        self.assertAlmostEqual(xyzOrigin.z() + 10.0, xyz.z(), delta=1e-6)

        # Decrease altitude
        xyz = sc.local_from_spherical_position(CoordinateVector3.spherical(
            lat, lon, elev - 10.0))
        self.assertIsNotNone(xyz)
        self.assertTrue(xyz.is_metric())
        self.assertAlmostEqual(xyzOrigin.x(), xyz.x(), delta=1e-6)
        self.assertAlmostEqual(xyzOrigin.y(), xyz.y(), delta=1e-6)
        self.assertAlmostEqual(xyzOrigin.z() - 10.0, xyz.z(), delta=1e-6)

        # Check how global and local velocities are connected

        # Velocity in
        # +X (East), +Y (North), -X (West), -Y (South), +Z (up), -Z (down)
        for global_var in [Vector3d.UNIT_X, Vector3d.UNIT_Y, Vector3d.UNIT_Z,
                           -Vector3d.UNIT_X, -Vector3d.UNIT_Y, -Vector3d.UNIT_Z]:
            local = sc.local_from_global_velocity(
                CoordinateVector3.metric(global_var))
            self.assertIsNotNone(local)
            self.assertTrue(local.is_metric())
            self.assertEqual(global_var, local.as_metric_vector())

            global_var = sc.global_from_local_velocity(local)
            self.assertIsNotNone(global_var)
            self.assertTrue(global_var.is_metric())
            self.assertEqual(global_var, local)

            # Directly call velocity_transform
            global_var = sc.velocity_transform(
                local,
                SphericalCoordinates.LOCAL,
                SphericalCoordinates.GLOBAL)
            self.assertIsNotNone(global_var)
            self.assertTrue(global_var.is_metric())
            self.assertEqual(global_var, local)

    @ignore_deprecation_warnings
    def test_no_heading_deprecated(self):
        # Default heading
        st = SphericalCoordinates.EARTH_WGS84
        lat = Angle(-22.9 * math.pi / 180.0)
        lon = Angle(-43.2 * math.pi / 180.0)
        heading = Angle(0.0)
        elev = 0
        sc = SphericalCoordinates(st, lat, lon, elev, heading)

        # Origin matches input
        latLonAlt = sc.spherical_from_local_position(Vector3d(0, 0, 0))
        self.assertEqual(lat.degree(), latLonAlt.x())
        self.assertEqual(lon.degree(), latLonAlt.y())
        self.assertEqual(elev, latLonAlt.z())

        xyzOrigin = sc.local_from_spherical_position(latLonAlt)
        self.assertEqual(Vector3d.ZERO, xyzOrigin)

        # Check how different lat/lon affect the local position

        # Increase latitude == go North == go +Y
        xyz = sc.local_from_spherical_position(
            Vector3d(lat.degree() + 1.0, lon.degree(), elev))
        self.assertAlmostEqual(xyzOrigin.x(), xyz.x(), delta=1e-6)
        self.assertLess(xyzOrigin.y(), xyz.y())

        # Decrease latitude == go South == go -Y
        xyz = sc.local_from_spherical_position(
            Vector3d(lat.degree() - 1.0, lon.degree(), elev))
        self.assertAlmostEqual(xyzOrigin.x(), xyz.x(), delta=1e-6)
        self.assertGreater(xyzOrigin.y(), xyz.y())

        # Increase longitude == go East == go +X
        # Also move a bit -Y because this is the Southern Hemisphere
        xyz = sc.local_from_spherical_position(
            Vector3d(lat.degree(), lon.degree() + 1.0, elev))
        self.assertLess(xyzOrigin.x(), xyz.x())
        self.assertGreater(xyzOrigin.y(), xyz.y())

        # Decrease longitude == go West == go -X
        # Also move a bit -Y because this is the Southern Hemisphere
        xyz = sc.local_from_spherical_position(
            Vector3d(lat.degree(), lon.degree() - 1.0, elev))
        self.assertGreater(xyzOrigin.x(), xyz.x())
        self.assertGreater(xyzOrigin.y(), xyz.y())

        # Increase altitude
        xyz = sc.local_from_spherical_position(
            Vector3d(lat.degree(), lon.degree(), elev + 10.0))
        self.assertAlmostEqual(xyzOrigin.x(), xyz.x(), delta=1e-6)
        self.assertAlmostEqual(xyzOrigin.y(), xyz.y(), delta=1e-6)
        self.assertAlmostEqual(xyzOrigin.z() + 10.0, xyz.z(), delta=1e-6)

        # Decrease altitude
        xyz = sc.local_from_spherical_position(
            Vector3d(lat.degree(), lon.degree(), elev - 10.0))
        self.assertAlmostEqual(xyzOrigin.x(), xyz.x(), delta=1e-6)
        self.assertAlmostEqual(xyzOrigin.y(), xyz.y(), delta=1e-6)
        self.assertAlmostEqual(xyzOrigin.z() - 10.0, xyz.z(), delta=1e-6)

        # Check how global and local velocities are connected

        # Velocity in
        # +X (East), +Y (North), -X (West), -Y (South), +Z (up), -Z (down)
        for global_var in [Vector3d.UNIT_X, Vector3d.UNIT_Y, Vector3d.UNIT_Z,
                           -Vector3d.UNIT_X, -Vector3d.UNIT_Y, -Vector3d.UNIT_Z]:
            local = sc.local_from_global_velocity(global_var)
            self.assertEqual(global_var, local)

            # This function is broken for horizontal velocities
            global_var = sc.global_from_local_velocity(local)
            wsu = CoordinateVector3.metric(
                -global_var.x(), -global_var.y(), global_var.z())
            self.assertAlmostEqual(wsu.x(), local.x(), delta=1e-6)
            self.assertAlmostEqual(wsu.y(), local.y(), delta=1e-6)
            self.assertAlmostEqual(wsu.z(), local.z(), delta=1e-6)

            # Directly call velocity_transform
            global_var = sc.velocity_transform(
                local,
                SphericalCoordinates.LOCAL,
                SphericalCoordinates.GLOBAL)
            wsu = CoordinateVector3.metric(
                -global_var.x(), -global_var.y(), global_var.z())
            self.assertAlmostEqual(wsu.x(), local.x(), delta=1e-6)
            self.assertAlmostEqual(wsu.y(), local.y(), delta=1e-6)
            self.assertAlmostEqual(wsu.z(), local.z(), delta=1e-6)

    def test_with_heading(self):
        # Heading 90 deg: X == North, Y == West , Z == Up
        st = SphericalCoordinates.EARTH_WGS84
        lat = Angle(-22.9 * math.pi / 180.0)
        lon = Angle(-43.2 * math.pi / 180.0)
        heading = Angle(90.0 * math.pi / 180.0)
        elev = 0
        sc = SphericalCoordinates(st, lat, lon, elev, heading)

        # Origin matches input
        latLonAlt = sc.spherical_from_local_position(
            CoordinateVector3.metric(0, 0, 0))
        self.assertIsNotNone(latLonAlt)
        self.assertTrue(latLonAlt.is_spherical())
        self.assertEqual(lat.degree(), latLonAlt.lat().degree())
        self.assertEqual(lon.degree(), latLonAlt.lon().degree())
        self.assertEqual(elev, latLonAlt.z())

        xyzOrigin = sc.local_from_spherical_position(latLonAlt)
        self.assertIsNotNone(xyzOrigin)
        self.assertTrue(xyzOrigin.is_metric())
        self.assertEqual(Vector3d.ZERO, xyzOrigin.as_metric_vector())

        # Check how different lat/lon affect the local position

        # Increase latitude == go North == go +X
        xyz = sc.local_from_spherical_position(CoordinateVector3.spherical(
            lat + Angle(math.radians(1.0)), lon, elev))
        self.assertIsNotNone(xyz)
        self.assertTrue(xyz.is_metric())
        self.assertAlmostEqual(xyzOrigin.y(), xyz.y(), delta=1e-6)
        self.assertLess(xyzOrigin.x(), xyz.x())

        # Decrease latitude == go South == go -X
        xyz = sc.local_from_spherical_position(CoordinateVector3.spherical(
            lat - Angle(math.radians(1.0)), lon, elev))
        self.assertIsNotNone(xyz)
        self.assertTrue(xyz.is_metric())
        self.assertAlmostEqual(xyzOrigin.y(), xyz.y(), delta=1e-6)
        self.assertGreater(xyzOrigin.x(), xyz.x())

        # Increase longitude == go East == go -Y (and a bit -X)
        xyz = sc.local_from_spherical_position(CoordinateVector3.spherical(
            lat, lon + Angle(math.radians(1.0)), elev))
        self.assertIsNotNone(xyz)
        self.assertTrue(xyz.is_metric())
        self.assertGreater(xyzOrigin.y(), xyz.y())
        self.assertGreater(xyzOrigin.x(), xyz.x())

        # Decrease longitude == go West == go +Y (and a bit -X)
        xyz = sc.local_from_spherical_position(CoordinateVector3.spherical(
            lat, lon - Angle(math.radians(1.0)), elev))
        self.assertIsNotNone(xyz)
        self.assertTrue(xyz.is_metric())
        self.assertLess(xyzOrigin.y(), xyz.y())
        self.assertGreater(xyzOrigin.x(), xyz.x())

        # Check how global and local velocities are connected

        # Global     | Local
        # ---------- | ------
        # +X (East)  | -Y
        # -X (West)  | +Y
        # +Y (North) | +X
        # -Y (South) | -X
        globalLocal = [
            [Vector3d.UNIT_X, -Vector3d.UNIT_Y],
            [-Vector3d.UNIT_X, Vector3d.UNIT_Y],
            [Vector3d.UNIT_Y, Vector3d.UNIT_X],
            [-Vector3d.UNIT_Y, -Vector3d.UNIT_X]]
        for [global_var, local] in globalLocal:
            localRes = sc.local_from_global_velocity(
                CoordinateVector3.metric(global_var))
            self.assertIsNotNone(localRes)
            self.assertTrue(localRes.is_metric())
            self.assertEqual(local, localRes.as_metric_vector())

            # Directly call velocity_transform
            globalRes = sc.velocity_transform(
                CoordinateVector3.metric(local),
                SphericalCoordinates.LOCAL,
                SphericalCoordinates.GLOBAL)
            self.assertIsNotNone(globalRes)
            self.assertTrue(globalRes.is_metric())
            self.assertEqual(global_var, globalRes.as_metric_vector())

    # TODO(peci1): Remove this test in Gazebo 9
    @ignore_deprecation_warnings
    def test_with_heading_deprecated(self):
        # Heading 90 deg: X == North, Y == West , Z == Up
        st = SphericalCoordinates.EARTH_WGS84
        lat = Angle(-22.9 * math.pi / 180.0)
        lon = Angle(-43.2 * math.pi / 180.0)
        heading = Angle(90.0 * math.pi / 180.0)
        elev = 0
        sc = SphericalCoordinates(st, lat, lon, elev, heading)

        # Origin matches input
        latLonAlt = sc.spherical_from_local_position(Vector3d(0, 0, 0))
        self.assertEqual(lat.degree(), latLonAlt.x())
        self.assertEqual(lon.degree(), latLonAlt.y())
        self.assertEqual(elev, latLonAlt.z())

        xyzOrigin = sc.local_from_spherical_position(latLonAlt)
        self.assertEqual(Vector3d.ZERO, xyzOrigin)

        # Check how different lat/lon affect the local position

        # Increase latitude == go North == go +X
        xyz = sc.local_from_spherical_position(
            Vector3d(lat.degree() + 1.0, lon.degree(), elev))
        self.assertAlmostEqual(xyzOrigin.y(), xyz.y(), delta=1e-6)
        self.assertLess(xyzOrigin.x(), xyz.x())

        # Decrease latitude == go South == go -X
        xyz = sc.local_from_spherical_position(
            Vector3d(lat.degree() - 1.0, lon.degree(), elev))
        self.assertAlmostEqual(xyzOrigin.y(), xyz.y(), delta=1e-6)
        self.assertGreater(xyzOrigin.x(), xyz.x())

        # Increase longitude == go East == go -Y (and a bit -X)
        xyz = sc.local_from_spherical_position(
            Vector3d(lat.degree(), lon.degree() + 1.0, elev))
        self.assertGreater(xyzOrigin.y(), xyz.y())
        self.assertGreater(xyzOrigin.x(), xyz.x())

        # Decrease longitude == go West == go +Y (and a bit -X)
        xyz = sc.local_from_spherical_position(
            Vector3d(lat.degree(), lon.degree() - 1.0, elev))
        self.assertLess(xyzOrigin.y(), xyz.y())
        self.assertGreater(xyzOrigin.x(), xyz.x())

        # Check how global and local velocities are connected

        # Global     | Local
        # ---------- | ------
        # +X (East)  | -Y
        # -X (West)  | +Y
        # +Y (North) | +X
        # -Y (South) | -X
        globalLocal = [
            [Vector3d.UNIT_X, -Vector3d.UNIT_Y],
            [-Vector3d.UNIT_X, Vector3d.UNIT_Y],
            [Vector3d.UNIT_Y, Vector3d.UNIT_X],
            [-Vector3d.UNIT_Y, -Vector3d.UNIT_X]]
        for [global_var, local] in globalLocal:
            localRes = sc.local_from_global_velocity(global_var)
            self.assertEqual(local, localRes)

            # Directly call fixed version
            globalRes = sc.velocity_transform(
                local,
                SphericalCoordinates.LOCAL,
                SphericalCoordinates.GLOBAL)
            self.assertEqual(global_var, globalRes)

    def test_inverse(self):
        st = SphericalCoordinates.EARTH_WGS84
        lat = Angle(0.3)
        lon = Angle(-1.2)
        heading = Angle(0.5)
        elev = 354.1
        sc = SphericalCoordinates(st, lat, lon, elev, heading)

        # GLOBAL <-> LOCAL
        in_vector = CoordinateVector3.metric(1, 2, -4)
        out = sc.velocity_transform(
            in_vector,
            SphericalCoordinates.LOCAL,
            SphericalCoordinates.GLOBAL)
        self.assertIsNotNone(out)
        self.assertTrue(out.is_metric())
        self.assertNotEqual(in_vector, out)
        reverse = sc.velocity_transform(
            out,
            SphericalCoordinates.GLOBAL,
            SphericalCoordinates.LOCAL)
        self.assertIsNotNone(reverse)
        self.assertTrue(reverse.is_metric())
        self.assertEqual(in_vector, reverse)

        in_vector = CoordinateVector3.metric(1, 2, -4)
        out = sc.position_transform(
            in_vector,
            SphericalCoordinates.LOCAL,
            SphericalCoordinates.GLOBAL)
        self.assertIsNotNone(out)
        self.assertTrue(out.is_metric())
        self.assertNotEqual(in_vector, out)
        reverse = sc.position_transform(
            out,
            SphericalCoordinates.GLOBAL,
            SphericalCoordinates.LOCAL)
        self.assertIsNotNone(reverse)
        self.assertTrue(reverse.is_metric())
        self.assertEqual(in_vector, reverse)

        # SPHERICAL <-> LOCAL
        in_vector = CoordinateVector3.metric(1, 2, -4)
        out = sc.position_transform(
            in_vector,
            SphericalCoordinates.LOCAL,
            SphericalCoordinates.SPHERICAL)
        self.assertIsNotNone(out)
        self.assertTrue(out.is_spherical())
        self.assertNotEqual(in_vector, out)
        reverse = sc.position_transform(
            out,
            SphericalCoordinates.SPHERICAL,
            SphericalCoordinates.LOCAL)
        self.assertIsNotNone(reverse)
        self.assertTrue(reverse.is_metric())
        self.assertEqual(in_vector, reverse)

    # TODO(peci1): Remove this test in Gazebo 9
    @ignore_deprecation_warnings
    def test_inverse_deprecated(self):
        st = SphericalCoordinates.EARTH_WGS84
        lat = Angle(0.3)
        lon = Angle(-1.2)
        heading = Angle(0.5)
        elev = 354.1
        sc = SphericalCoordinates(st, lat, lon, elev, heading)

        # GLOBAL <-> LOCAL2
        in_vector = Vector3d(1, 2, -4)
        out = sc.velocity_transform(
            in_vector,
            SphericalCoordinates.LOCAL2,
            SphericalCoordinates.GLOBAL)
        self.assertNotEqual(in_vector, out)
        reverse = sc.velocity_transform(
            out,
            SphericalCoordinates.GLOBAL,
            SphericalCoordinates.LOCAL2)
        self.assertEqual(in_vector, reverse)

        in_vector = Vector3d(1, 2, -4)
        out = sc.position_transform(
            in_vector,
            SphericalCoordinates.LOCAL2,
            SphericalCoordinates.GLOBAL)
        self.assertNotEqual(in_vector, out)
        reverse = sc.position_transform(
            out,
            SphericalCoordinates.GLOBAL,
            SphericalCoordinates.LOCAL2)
        self.assertEqual(in_vector, reverse)

        # SPHERICAL <-> LOCAL2
        in_vector = Vector3d(1, 2, -4)
        out = sc.position_transform(
            in_vector,
            SphericalCoordinates.LOCAL2,
            SphericalCoordinates.SPHERICAL)
        self.assertNotEqual(in_vector, out)
        reverse = sc.position_transform(
            out,
            SphericalCoordinates.SPHERICAL,
            SphericalCoordinates.LOCAL2)
        self.assertEqual(in_vector, reverse)

        # GLOBAL <-> LOCAL
        in_vector = Vector3d(1, 2, -4)
        out = sc.velocity_transform(
            in_vector,
            SphericalCoordinates.LOCAL,
            SphericalCoordinates.GLOBAL)
        self.assertNotEqual(in_vector, out)
        reverse = sc.velocity_transform(
            out,
            SphericalCoordinates.GLOBAL,
            SphericalCoordinates.LOCAL)
        self.assertNotEqual(in_vector, reverse)

        in_vector = Vector3d(1, 2, -4)
        out = sc.position_transform(
            in_vector,
            SphericalCoordinates.LOCAL,
            SphericalCoordinates.GLOBAL)
        self.assertNotEqual(in_vector, out)
        reverse = sc.position_transform(
            out,
            SphericalCoordinates.GLOBAL,
            SphericalCoordinates.LOCAL)
        self.assertNotEqual(in_vector, reverse)

        # SPHERICAL <-> LOCAL
        in_vector = Vector3d(1, 2, -4)
        out = sc.position_transform(
            in_vector,
            SphericalCoordinates.LOCAL,
            SphericalCoordinates.SPHERICAL)
        self.assertNotEqual(in_vector, out)
        reverse = sc.position_transform(
            out,
            SphericalCoordinates.SPHERICAL,
            SphericalCoordinates.LOCAL)
        self.assertNotEqual(in_vector, reverse)


if __name__ == '__main__':
    unittest.main()
