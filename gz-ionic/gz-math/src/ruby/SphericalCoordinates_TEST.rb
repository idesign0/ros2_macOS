# Copyright (C) 2024 Open Source Robotics Foundation
#
# Licensed under the Apache License, Version 2.0 (the "License")
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#!/usr/bin/env ruby

require 'test/unit/ui/console/testrunner'
require 'test/unit'
require 'math'

class SphericalCoordinates_TEST < Test::Unit::TestCase
  def test_construction
    v = Gz::Math::SphericalCoordinates.new
    assert(v.Surface() == Gz::Math::SphericalCoordinates::EARTH_WGS84, "Wrong Surface()")
    assert(v.LatitudeReference() == Gz::Math::Angle.new(), "Wrong LatitudeReference()")
    assert(v.LongitudeReference() == Gz::Math::Angle.new(), "Wrong LongitudeReference()")
    assert(v.HeadingOffset() == Gz::Math::Angle.new(), "Wrong HeadingOffset()")
    assert(v.ElevationReference() == 0.0, "Wrong ElevationReference()")

    v = Gz::Math::SphericalCoordinates.new(Gz::Math::SphericalCoordinates::EARTH_WGS84)
    assert(v.Surface() == Gz::Math::SphericalCoordinates::EARTH_WGS84, "Wrong Surface()")
    assert(v.LatitudeReference() == Gz::Math::Angle.new(), "Wrong LatitudeReference()")
    assert(v.LongitudeReference() == Gz::Math::Angle.new(), "Wrong LongitudeReference()")
    assert(v.HeadingOffset() == Gz::Math::Angle.new(), "Wrong HeadingOffset()")
    assert(v.ElevationReference() == 0.0, "Wrong ElevationReference()")

    lat = Gz::Math::Angle.new(0.3)
    lon = Gz::Math::Angle.new(-1.2)
    heading = Gz::Math::Angle.new(0.5)
    elev = 354.1
    v = Gz::Math::SphericalCoordinates.new(
      Gz::Math::SphericalCoordinates::EARTH_WGS84, lat, lon, elev, heading)
    assert(v.Surface() == Gz::Math::SphericalCoordinates::EARTH_WGS84, "Wrong Surface()")
    assert(v.LatitudeReference() == lat, "Wrong LatitudeReference()")
    assert(v.LongitudeReference() == lon, "Wrong LongitudeReference()")
    assert(v.HeadingOffset() == heading, "Wrong HeadingOffset()")
    assert(v.ElevationReference() == elev, "Wrong ElevationReference()")

    v2 = Gz::Math::SphericalCoordinates.new(v)
    assert(v2 == v, "instances should equal")

    # TODO(anyone): std::optional<> bindings are not working,
    #               so this test doesn't test much
  end

end

exit Test::Unit::UI::Console::TestRunner.run(SphericalCoordinates_TEST).passed? ? 0 : -1
