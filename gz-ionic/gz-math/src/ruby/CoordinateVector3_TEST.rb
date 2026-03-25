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

class CoordinateVector3_TEST < Test::Unit::TestCase
  def test_construction
    v = Gz::Math::CoordinateVector3.new
    assert(v.IsMetric(), "CoordinateVector3::IsMetric() should be true")
    assert(!v.IsSpherical(), "CoordinateVector3::IsSpherical() should be false")
    assert(v.IsFinite(), "CoordinateVector3::IsFinite() should be true")
    # TODO(anyone): std::optional<> bindings are not working,
    #               so this test doesn't test much
    # assert(v.X() == 0.0, "x should be 0")
    # assert(v.Y() == 0.0, "y should be 0")
    # assert(v.Z() == 0.0, "z should be 0")

    v = Gz::Math::CoordinateVector3.Metric(1.0, 2.0, 3.0)
    assert(v.IsMetric(), "CoordinateVector3::IsMetric() should be true")
    assert(!v.IsSpherical(), "CoordinateVector3::IsSpherical() should be false")
    assert(v.IsFinite(), "CoordinateVector3::IsFinite() should be true")

    v = Gz::Math::CoordinateVector3.Metric(Gz::Math::Vector3d.new(1.0, 2.0, 3.0))
    assert(v.IsMetric(), "CoordinateVector3::IsMetric() should be true")
    assert(!v.IsSpherical(), "CoordinateVector3::IsSpherical() should be false")
    assert(v.IsFinite(), "CoordinateVector3::IsFinite() should be true")

    v = Gz::Math::CoordinateVector3.Spherical(
      Gz::Math::Angle.new(1.0), Gz::Math::Angle.new(2.0), 3.0)
    assert(!v.IsMetric(), "CoordinateVector3::IsMetric() should be false")
    assert(v.IsSpherical(), "CoordinateVector3::IsSpherical() should be true")
    assert(v.IsFinite(), "CoordinateVector3::IsFinite() should be true")
  end

end

exit Test::Unit::UI::Console::TestRunner.run(CoordinateVector3_TEST).passed? ? 0 : -1
