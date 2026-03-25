#!/usr/bin/env python
#*********************************************************************
# Software License Agreement (BSD License)
#
#  Copyright (c) 2015, Bossa Nova Robotics
#  All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:
#
#   * Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#   * Redistributions in binary form must reproduce the above
#     copyright notice, this list of conditions and the following
#     disclaimer in the documentation and/or other materials provided
#     with the distribution.
#   * Neither the name of the Bossa Nova Robotics nor the names of its
#     contributors may be used to endorse or promote products derived
#     from this software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
#  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
#  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
#  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
#  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES
#  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
#  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
#  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
#  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
#  POSSIBILITY OF SUCH DAMAGE.
#********************************************************************/
from angles import normalize_angle_positive, normalize_angle, shortest_angular_distance, two_pi_complement, shortest_angular_distance_with_limits, shortest_angular_distance_with_large_limits
from angles import _find_min_max_delta
import sys
import unittest
from math import pi, fabs

## A sample python unit test
class TestAngles(unittest.TestCase):
    def test_shortestDistanceWithLimits(self):
        result, shortest_angle = shortest_angular_distance_with_limits(-0.5, 0.5,-0.25,0.25)
        self.assertFalse(result)

        result, shortest_angle = shortest_angular_distance_with_limits(-0.5, 0.5,0.25,0.25)
        self.assertFalse(result)

        result, shortest_angle = shortest_angular_distance_with_limits(-0.5, 0.5,0.25,-0.25)
        self.assertTrue(result)
        self.assertAlmostEqual(shortest_angle, -2*pi+1.0)

        result, shortest_angle = shortest_angular_distance_with_limits(0.5, 0.5,0.25,-0.25)
        self.assertTrue(result)
        self.assertAlmostEqual(shortest_angle, 0)

        result, shortest_angle = shortest_angular_distance_with_limits(0.5, 0,0.25,-0.25)
        self.assertFalse(result)
        self.assertAlmostEqual(shortest_angle, -0.5)

        result, shortest_angle = shortest_angular_distance_with_limits(-0.5, 0,0.25,-0.25)
        self.assertFalse(result)
        self.assertAlmostEqual(shortest_angle, 0.5)

        result, shortest_angle = shortest_angular_distance_with_limits(-0.2,0.2,0.25,-0.25)
        self.assertFalse(result)
        self.assertAlmostEqual(shortest_angle, -2*pi+0.4)

        result, shortest_angle = shortest_angular_distance_with_limits(0.2,-0.2,0.25,-0.25)
        self.assertFalse(result)
        self.assertAlmostEqual(shortest_angle,2*pi-0.4)

        result, shortest_angle = shortest_angular_distance_with_limits(0.2,0,0.25,-0.25)
        self.assertFalse(result)
        self.assertAlmostEqual(shortest_angle,2*pi-0.2)

        result, shortest_angle = shortest_angular_distance_with_limits(-0.2,0,0.25,-0.25)
        self.assertFalse(result)
        self.assertAlmostEqual(shortest_angle,-2*pi+0.2)

        result, shortest_angle = shortest_angular_distance_with_limits(-0.25,-0.5,0.25,-0.25)
        self.assertTrue(result)
        self.assertAlmostEqual(shortest_angle,-0.25)

        result, shortest_angle = shortest_angular_distance_with_limits(-0.25,0.5,0.25,-0.25)
        self.assertTrue(result)
        self.assertAlmostEqual(shortest_angle,-2*pi+0.75)

        result, shortest_angle = shortest_angular_distance_with_limits(-0.2500001,0.5,0.25,-0.25)
        self.assertTrue(result)
        self.assertAlmostEqual(shortest_angle,-2*pi+0.5+0.2500001)

        result, shortest_angle = shortest_angular_distance_with_limits(-0.6, 0.5,-0.25,0.25)
        self.assertFalse(result)

        result, shortest_angle = shortest_angular_distance_with_limits(-0.5, 0.6,-0.25,0.25)
        self.assertFalse(result)

        result, shortest_angle = shortest_angular_distance_with_limits(-0.6, 0.75,-0.25,0.3)
        self.assertFalse(result)

        result, shortest_angle = shortest_angular_distance_with_limits(-0.6, pi*3.0/4.0,-0.25,0.3)
        self.assertFalse(result)

        result, shortest_angle = shortest_angular_distance_with_limits(-pi, pi,-pi,pi)
        self.assertTrue(result)
        self.assertAlmostEqual(shortest_angle,0.0)
    
    def test_shortestDistanceWithLargeLimits(self):
        # 'delta' is valid
        result, shortest_angle = shortest_angular_distance_with_large_limits(0, 10.5*pi, -2*pi, 2*pi)
        self.assertTrue(result)
        self.assertAlmostEqual(shortest_angle, 0.5*pi)

        # 'delta' is not valid, but 'delta_2pi' is
        result, shortest_angle = shortest_angular_distance_with_large_limits(0, 10.5*pi, -2*pi, 0.1*pi)
        self.assertTrue(result)
        self.assertAlmostEqual(shortest_angle, -1.5*pi)

        # neither 'delta' nor 'delta_2pi' are valid
        result, shortest_angle = shortest_angular_distance_with_large_limits(2*pi, pi, 2*pi-0.1, 2*pi+0.1)
        self.assertFalse(result)

        # start position outside limits
        result, shortest_angle = shortest_angular_distance_with_large_limits(10.5*pi, 0, -2*pi, 2*pi)
        self.assertFalse(result)

        # invalid limits (lower > upper)
        result, shortest_angle = shortest_angular_distance_with_large_limits(0, 0.1, 2*pi, -2*pi)
        self.assertFalse(result)

        # specific test case
        result, shortest_angle = shortest_angular_distance_with_large_limits(0.999507, 1.0, -20*pi, 20*pi)
        self.assertTrue(result)
        self.assertAlmostEqual(shortest_angle, 0.000493)

    def test_normalize_angle_positive(self):
        self.assertAlmostEqual(0, normalize_angle_positive(0))
        self.assertAlmostEqual(pi, normalize_angle_positive(pi))
        self.assertAlmostEqual(0, normalize_angle_positive(2*pi))
        self.assertAlmostEqual(pi, normalize_angle_positive(3*pi))
        self.assertAlmostEqual(0, normalize_angle_positive(4*pi))

        self.assertAlmostEqual(0, normalize_angle_positive(-0))
        self.assertAlmostEqual(pi, normalize_angle_positive(-pi))
        self.assertAlmostEqual(0, normalize_angle_positive(-2*pi))
        self.assertAlmostEqual(pi, normalize_angle_positive(-3*pi))
        self.assertAlmostEqual(0, normalize_angle_positive(-4*pi))

        self.assertAlmostEqual(0, normalize_angle_positive(-0))
        self.assertAlmostEqual(3*pi/2, normalize_angle_positive(-pi/2))
        self.assertAlmostEqual(pi, normalize_angle_positive(-pi))
        self.assertAlmostEqual(pi/2, normalize_angle_positive(-3*pi/2))
        self.assertAlmostEqual(0, normalize_angle_positive(-4*pi/2))

        self.assertAlmostEqual(0, normalize_angle_positive(0))
        self.assertAlmostEqual(pi/2, normalize_angle_positive(pi/2))
        self.assertAlmostEqual(pi/2, normalize_angle_positive(5*pi/2))
        self.assertAlmostEqual(pi/2, normalize_angle_positive(9*pi/2))
        self.assertAlmostEqual(pi/2, normalize_angle_positive(-3*pi/2))

    def test_normalize_angle(self):
        self.assertAlmostEqual(0, normalize_angle(0))
        self.assertAlmostEqual(pi, normalize_angle(pi))
        self.assertAlmostEqual(0, normalize_angle(2*pi))
        self.assertAlmostEqual(pi, normalize_angle(3*pi))
        self.assertAlmostEqual(0, normalize_angle(4*pi))

        self.assertAlmostEqual(0, normalize_angle(-0))
        self.assertAlmostEqual(pi, normalize_angle(-pi))
        self.assertAlmostEqual(0, normalize_angle(-2*pi))
        self.assertAlmostEqual(pi, normalize_angle(-3*pi))
        self.assertAlmostEqual(0, normalize_angle(-4*pi))

        self.assertAlmostEqual(0, normalize_angle(-0))
        self.assertAlmostEqual(-pi/2, normalize_angle(-pi/2))
        self.assertAlmostEqual(pi, normalize_angle(-pi))
        self.assertAlmostEqual(pi/2, normalize_angle(-3*pi/2))
        self.assertAlmostEqual(0, normalize_angle(-4*pi/2))

        self.assertAlmostEqual(0, normalize_angle(0))
        self.assertAlmostEqual(pi/2, normalize_angle(pi/2))
        self.assertAlmostEqual(pi/2, normalize_angle(5*pi/2))
        self.assertAlmostEqual(pi/2, normalize_angle(9*pi/2))
        self.assertAlmostEqual(pi/2, normalize_angle(-3*pi/2))
 
    def test_shortest_angular_distance(self):
         self.assertAlmostEqual(pi/2, shortest_angular_distance(0, pi/2))
         self.assertAlmostEqual(-pi/2, shortest_angular_distance(0, -pi/2))
         self.assertAlmostEqual(-pi/2, shortest_angular_distance(pi/2, 0))
         self.assertAlmostEqual(pi/2, shortest_angular_distance(-pi/2, 0))

         self.assertAlmostEqual(-pi/2, shortest_angular_distance(pi, pi/2))
         self.assertAlmostEqual(pi/2, shortest_angular_distance(pi, -pi/2))
         self.assertAlmostEqual(pi/2, shortest_angular_distance(pi/2, pi))
         self.assertAlmostEqual(-pi/2, shortest_angular_distance(-pi/2, pi))

         self.assertAlmostEqual(-pi/2, shortest_angular_distance(5*pi, pi/2))
         self.assertAlmostEqual(pi/2, shortest_angular_distance(7*pi, -pi/2))
         self.assertAlmostEqual(pi/2, shortest_angular_distance(9*pi/2, pi))
         self.assertAlmostEqual(pi/2, shortest_angular_distance(-3*pi/2, pi))

         # Backside wrapping
         self.assertAlmostEqual(-pi/2, shortest_angular_distance(-3*pi/4, 3*pi/4))
         self.assertAlmostEqual(pi/2, shortest_angular_distance(3*pi/4, -3*pi/4))

    def test_two_pi_complement(self):
         epsilon = 1e-9
         self.assertAlmostEqual(two_pi_complement(0), 2*pi)
         self.assertAlmostEqual(two_pi_complement(2*pi), 0)
         self.assertAlmostEqual(two_pi_complement(-2*pi), 0)
         self.assertAlmostEqual(two_pi_complement(2*pi-epsilon), -epsilon)
         self.assertAlmostEqual(two_pi_complement(-2*pi+epsilon), epsilon)
         self.assertAlmostEqual(two_pi_complement(pi/2), -3*pi/2)
         self.assertAlmostEqual(two_pi_complement(pi), -pi)
         self.assertAlmostEqual(two_pi_complement(-pi), pi)
         self.assertAlmostEqual(two_pi_complement(-pi/2), 3*pi/2)

         self.assertAlmostEqual(two_pi_complement(3*pi), -pi)
         self.assertAlmostEqual(two_pi_complement(-3.0*pi), pi)
         self.assertAlmostEqual(two_pi_complement(-5.0*pi/2.0), 3*pi/2)

    def test_find_min_max_delta(self):
         epsilon = 1e-9
         # Straight forward full range
         flag, min_delta, max_delta = _find_min_max_delta( 0, -pi, pi)
         self.assertTrue(flag)
         self.assertAlmostEqual(min_delta, -pi)
         self.assertAlmostEqual(max_delta, pi)

         # pi/2 Full Range
         flag, min_delta, max_delta = _find_min_max_delta( pi/2, -pi, pi)
         self.assertTrue(flag)
         self.assertAlmostEqual(min_delta, -3*pi/2)
         self.assertAlmostEqual(max_delta, pi/2)

         # -pi/2 Full range
         flag, min_delta, max_delta = _find_min_max_delta( -pi/2, -pi, pi)
         self.assertTrue(flag)
         self.assertAlmostEqual(min_delta, -pi/2)
         self.assertAlmostEqual(max_delta, 3*pi/2)

         # Straight forward partial range
         flag, min_delta, max_delta = _find_min_max_delta( 0, -pi/2, pi/2)
         self.assertTrue(flag)
         self.assertAlmostEqual(min_delta, -pi/2)
         self.assertAlmostEqual(max_delta, pi/2)

         # pi/4 Partial Range
         flag, min_delta, max_delta = _find_min_max_delta( pi/4, -pi/2, pi/2)
         self.assertTrue(flag)
         self.assertAlmostEqual(min_delta, -3*pi/4)
         self.assertAlmostEqual(max_delta, pi/4)

         # -pi/4 Partial Range
         flag, min_delta, max_delta = _find_min_max_delta( -pi/4, -pi/2, pi/2)
         self.assertTrue(flag)
         self.assertAlmostEqual(min_delta, -pi/4)
         self.assertAlmostEqual(max_delta, 3*pi/4)

         # bump stop negative full range
         flag, min_delta, max_delta = _find_min_max_delta( -pi, -pi, pi)
         self.assertTrue(flag)
         self.assertTrue((fabs(min_delta) <= epsilon and fabs(max_delta - 2*pi) <= epsilon) or (fabs(min_delta+2*pi) <= epsilon and fabs(max_delta) <= epsilon))
         self.assertAlmostEqual(min_delta, 0.0)
         self.assertAlmostEqual(max_delta, 2*pi)

         flag, min_delta, max_delta = _find_min_max_delta(-0.25,0.25,-0.25)
         self.assertTrue(flag)
         self.assertAlmostEqual(min_delta, -2*pi+0.5)
         self.assertAlmostEqual(max_delta, 0.0)

         # bump stop positive full range
         flag, min_delta, max_delta = _find_min_max_delta( pi-epsilon, -pi, pi)
         self.assertTrue(flag)
         #self.assertTrue((fabs(min_delta) <= epsilon and fabs(max_delta - 2*pi) <= epsilon) or (fabs(min_delta+2*pi) <= epsilon and fabs(max_delta) <= epsilon))
         self.assertAlmostEqual(min_delta, -2*pi+epsilon)
         self.assertAlmostEqual(max_delta, epsilon)

         # bump stop negative partial range
         flag, min_delta, max_delta = _find_min_max_delta( -pi, -pi, pi)
         self.assertTrue(flag)
         self.assertAlmostEqual(min_delta, 0)
         self.assertAlmostEqual(max_delta, 2*pi)

         # bump stop positive partial range
         flag, min_delta, max_delta = _find_min_max_delta( -pi/2, -pi/2, pi/2)
         self.assertTrue(flag)
         self.assertAlmostEqual(min_delta, 0.0)
         self.assertAlmostEqual(max_delta, pi)

         #Test out of range negative
         flag, min_delta, max_delta = _find_min_max_delta( -pi, -pi/2, pi/2)
         self.assertFalse(flag)
         #Test out of range postive
         flag, min_delta, max_delta = _find_min_max_delta( pi, -pi/2, pi/2)
         self.assertFalse(flag)

         # pi/4 Partial Range
         flag, min_delta, max_delta = _find_min_max_delta( 3*pi/4, pi/2, -pi/2)
         self.assertTrue(flag)
         self.assertAlmostEqual(min_delta, -pi/4)
         self.assertAlmostEqual(max_delta, 3*pi/4)

if __name__ == '__main__':
    unittest.main()
