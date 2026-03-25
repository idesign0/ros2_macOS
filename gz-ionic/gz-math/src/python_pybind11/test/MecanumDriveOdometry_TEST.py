# Copyright (C) 2023 Open Source Robotics Foundation
#
# Licensed under the Apache License, Version 2.0 (the "License")
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#       http:       #www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import datetime
import math
import time
import unittest

from gz.math8 import MecanumDriveOdometry, Angle


class TestMecanumDriveOdometry(unittest.TestCase):

    def test_constructor(self):
        odom = MecanumDriveOdometry()
        self.assertAlmostEqual(0.0, odom.heading().radian())
        self.assertAlmostEqual(0.0, odom.x())
        self.assertAlmostEqual(0.0, odom.y())
        self.assertAlmostEqual(0.0, odom.linear_velocity())
        self.assertAlmostEqual(0.0, odom.lateral_velocity())
        self.assertAlmostEqual(0.0, odom.angular_velocity().radian())

        wheelSeparation = 2.0
        wheelRadius = 0.5
        wheelCircumference = 2 * math.pi * wheelRadius

        # This is the linear distance traveled per degree of wheel rotation.
        distPerDegree = wheelCircumference / 360.0

        # Setup the wheel parameters, and initialize
        odom.set_wheel_params(wheelSeparation, wheelRadius, wheelRadius,wheelRadius)
        startTime = datetime.timedelta(time.monotonic())
        odom.init(startTime)

        # Sleep for a little while, then update the odometry with the new wheel
        # position.
        time1 = startTime + datetime.timedelta(milliseconds=100)
        odom.update(Angle(math.radians(1.0)),
                    Angle(math.radians(1.0)),
                    Angle(math.radians(1.0)),
                    Angle(math.radians(1.0)),
                    time1)
        self.assertAlmostEqual(0.0, odom.heading().radian())
        self.assertAlmostEqual(distPerDegree, odom.x())
        self.assertAlmostEqual(0.0, odom.y())
        # Linear velocity should be dist_traveled / time_elapsed.
        self.assertAlmostEqual(distPerDegree / 0.1, odom.linear_velocity(), delta=1e-3)
        # Angular velocity should be zero since the "robot" is traveling in a
        # straight line.
        self.assertAlmostEqual(0.0, odom.angular_velocity().radian(), delta=1e-3)

        # Sleep again, then update the odometry with the new wheel position.
        time2 = time1 + datetime.timedelta(milliseconds=100)
        odom.update(Angle(math.radians(2.0)),
                    Angle(math.radians(2.0)),
                    Angle(math.radians(2.0)),
                    Angle(math.radians(2.0)),
                    time2)
        self.assertAlmostEqual(0.0, odom.heading().radian())
        self.assertAlmostEqual(distPerDegree * 2.0, odom.x(), delta=3e-6)
        self.assertAlmostEqual(0.0, odom.y())
        # Linear velocity should be dist_traveled / time_elapsed.
        self.assertAlmostEqual(distPerDegree / 0.1, odom.linear_velocity(), delta=1e-3)
        # Angular velocity should be zero since the "robot" is traveling in a
        # straight line.
        self.assertAlmostEqual(0.0, odom.angular_velocity().radian(), delta=1e-3)

        # Initialize again, and odom values should be reset.
        startTime = datetime.timedelta(time.monotonic())
        odom.init(startTime)
        self.assertAlmostEqual(0.0, odom.heading().radian())
        self.assertAlmostEqual(0.0, odom.x())
        self.assertAlmostEqual(0.0, odom.y())
        self.assertAlmostEqual(0.0, odom.linear_velocity())
        self.assertAlmostEqual(0.0, odom.angular_velocity().radian())

        # Sleep again, this time move 2 degrees in 100ms.
        time1 = startTime + datetime.timedelta(milliseconds=100)
        odom.update(Angle(math.radians(2.0)),
                    Angle(math.radians(2.0)),
                    Angle(math.radians(2.0)),
                    Angle(math.radians(2.0)),
                    time1)
        self.assertAlmostEqual(0.0, odom.heading().radian())
        self.assertAlmostEqual(distPerDegree * 2.0, odom.x(), delta=3e-6)
        self.assertAlmostEqual(0.0, odom.y())
        # Linear velocity should be dist_traveled / time_elapsed.
        self.assertAlmostEqual(distPerDegree * 2 / 0.1, odom.linear_velocity(), delta=1e-3)
        # Angular velocity should be zero since the "robot" is traveling in a
        # straight line.
        self.assertAlmostEqual(0.0, odom.angular_velocity().radian(), delta=1e-3)

        # Sleep again, this time move 2 degrees in 100ms.
        odom.init(startTime)
        time1 = startTime + datetime.timedelta(milliseconds=100)
        odom.update(Angle(math.radians(-2.0)),
                    Angle(math.radians(2.0)),
                    Angle(math.radians(2.0)),
                    Angle(math.radians(-2.0)),
                    time1)
        self.assertAlmostEqual(0.0, odom.heading().radian())
        self.assertAlmostEqual(distPerDegree * 2.0, odom.y(), delta=3e-6)
        # self.assertAlmostEqual(0.0, odom.y())
        # Linear velocity should be dist_traveled / time_elapsed.
        self.assertAlmostEqual(distPerDegree * 2 / 0.1, odom.lateral_velocity(), delta=1e-3)
        # Angular velocity should be zero since the "robot" is traveling in a
        # straight line.
        self.assertAlmostEqual(0.0, odom.angular_velocity().radian(), delta=1e-3)


if __name__ == '__main__':
    unittest.main()
