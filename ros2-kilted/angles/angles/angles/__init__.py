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

from math import fmod, pi, fabs

def normalize_angle_positive(angle):
    """ Normalizes the angle to be 0 to 2*pi
        It takes and returns radians. """
    return angle % (2.0*pi)

def normalize_angle(angle):
    """ Normalizes the angle to be -pi to +pi
        It takes and returns radians."""
    a = normalize_angle_positive(angle)
    if a > pi:
        a -= 2.0 *pi
    return a

def shortest_angular_distance(from_angle, to_angle):
    """ Given 2 angles, this returns the shortest angular
        difference.  The inputs and ouputs are of course radians.
 
        The result would always be -pi <= result <= pi. Adding the result
        to "from" will always get you an equivelent angle to "to".
    """
    return normalize_angle(to_angle-from_angle)

def two_pi_complement(angle):
    """ returns the angle in [-2*pi, 2*pi]  going the other way along the unit circle.
        \param angle The angle to which you want to turn in the range [-2*pi, 2*pi]
            E.g. two_pi_complement(-pi/4) returns 7_pi/4
                 two_pi_complement(pi/4) returns -7*pi/4
    """
    #check input conditions
    if angle > 2*pi or angle < -2.0*pi:
        angle = fmod(angle, 2.0*pi)
    if angle < 0:
        return 2*pi+angle
    elif angle > 0:
        return -2*pi+angle

    return 2*pi

def _find_min_max_delta(from_angle, left_limit, right_limit):
    """ This function is only intended for internal use and not intended for external use. 
        If you do use it, read the documentation very carefully. 
        
        Returns the min and max amount (in radians) that can be moved 
        from "from" angle to "left_limit" and "right_limit".
        
        \param from - "from" angle - must lie in [-pi, pi)
        \param left_limit - left limit of valid interval for angular position 
            - must lie in [-pi, pi], left and right limits are specified on 
              the unit circle w.r.t to a reference pointing inwards
        \param right_limit - right limit of valid interval for angular position 
            - must lie in [-pi, pi], left and right limits are specified on 
              the unit circle w.r.t to a reference pointing inwards      
        \return (valid, min, max) - angle in radians that can be moved from "from" position before hitting the joint stop
                 valid is False  if "from" angle does not lie in the interval [left_limit,right_limit]
        """
    delta = [0]*4
    delta[0] = shortest_angular_distance(from_angle,left_limit)
    delta[1] = shortest_angular_distance(from_angle,right_limit)
    delta[2] = two_pi_complement(delta[0])
    delta[3] = two_pi_complement(delta[1])

    if delta[0] == 0:
        return True, delta[0], max(delta[1], delta[3])
    
    if delta[1] == 0:
        return True, min(delta[0], delta[2]), delta[1]

    delta_min = delta[0]
    delta_min_2pi = delta[2]
    if delta[2] < delta_min:
        delta_min = delta[2]
        delta_min_2pi = delta[0]
  
    delta_max = delta[1]
    delta_max_2pi = delta[3]
    if delta[3] > delta_max:
        delta_max = delta[3]
        delta_max_2pi = delta[1]

    # printf("%f %f %f %f\n",delta_min,delta_min_2pi,delta_max,delta_max_2pi)
    if (delta_min <= delta_max_2pi) or (delta_max >= delta_min_2pi):
        if left_limit == -pi and right_limit == pi:
            return (True, delta_max_2pi, delta_min_2pi)
        else:
            return (False, delta_max_2pi, delta_min_2pi)
    return True, delta_min, delta_max

def shortest_angular_distance_with_limits(from_angle, to_angle, left_limit, right_limit):
    """ Returns the delta from "from_angle" to "to_angle" making sure it does not violate limits specified by left_limit and right_limit.
        The valid interval of angular positions is [left_limit,right_limit]. E.g., [-0.25,0.25] is a 0.5 radians wide interval that contains 0.
        But [0.25,-0.25] is a 2*pi-0.5 wide interval that contains pi (but not 0).
        The value of shortest_angle is the angular difference between "from" and "to" that lies within the defined valid interval.
        
        E.g. shortest_angular_distance_with_limits(-0.5,0.5,0.25,-0.25) returns 2*pi-1.0
             shortest_angular_distance_with_limits(-0.5,0.5,-0.25,0.25) returns None since -0.5 and 0.5 do not lie in the interval [-0.25,0.25]
             
        \param left_limit - left limit of valid interval for angular position 
            - must lie in [-pi, pi], left and right limits are specified on 
              the unit circle w.r.t to a reference pointing inwards
        \param right_limit - right limit of valid interval for angular position 
            - must lie in [-pi, pi], left and right limits are specified on 
              the unit circle w.r.t to a reference pointing inwards   
        \returns valid_flag, shortest_angle 
    """
    min_delta = -2*pi
    max_delta = 2*pi
    min_delta_to = -2*pi
    max_delta_to = 2*pi
    flag, min_delta, max_delta = _find_min_max_delta(from_angle, left_limit, right_limit)
    delta = shortest_angular_distance(from_angle,to_angle)
    delta_mod_2pi  = two_pi_complement(delta)

    if flag: #from position is within the limits
        if delta >= min_delta and delta <= max_delta:
            return True, delta
        elif delta_mod_2pi >= min_delta and delta_mod_2pi <= max_delta:
          return True, delta_mod_2pi
        else: #to position is outside the limits
            flag, min_delta_to, max_delta_to = _find_min_max_delta(to_angle,left_limit,right_limit)
            if fabs(min_delta_to) < fabs(max_delta_to):
                shortest_angle = max(delta, delta_mod_2pi)
            elif fabs(min_delta_to) > fabs(max_delta_to):
                shortest_angle = min(delta,delta_mod_2pi)
            else:
                if fabs(delta) < fabs(delta_mod_2pi):
                    shortest_angle = delta
                else:
                    shortest_angle = delta_mod_2pi
            return False, shortest_angle
    else: # from position is outside the limits
        flag, min_delta_to, max_delta_to = _find_min_max_delta(to_angle,left_limit,right_limit)

        if fabs(min_delta) < fabs(max_delta):
            shortest_angle = min(delta,delta_mod_2pi)
        elif fabs(min_delta) > fabs(max_delta):
          shortest_angle = max(delta,delta_mod_2pi)
        else:
            if fabs(delta) < fabs(delta_mod_2pi):
                shortest_angle = delta
            else:
                shortest_angle = delta_mod_2pi
        return False, shortest_angle

def shortest_angular_distance_with_large_limits(from_angle, to_angle, left_limit, right_limit):
    """ Returns the delta from `from_angle` to `to_angle`, making sure it does not violate limits specified by `left_limit` and `right_limit`.
        This function is similar to `shortest_angular_distance_with_limits()`, with the main difference that it accepts limits outside the `[-M_PI, M_PI]` range.
        Even if this is quite uncommon, one could indeed consider revolute joints with large rotation limits, e.g., in the range `[-2*M_PI, 2*M_PI]`.

        In this case, a strict requirement is to have `left_limit` smaller than `right_limit`.
        Note also that `from_angle` must lie inside the valid range, while `to_angle` does not need to.
        In fact, this function will evaluate the shortest (valid) angle `shortest_angle` so that `from_angle+shortest_angle` equals `to_angle` up to an integer multiple of `2*M_PI`.
        As an example, a call to `shortest_angular_distance_with_large_limits(0, 10.5*M_PI, -2*M_PI, 2*M_PI)` will return `true`, with `shortest_angle=0.5*M_PI`.
        This is because `from_angle` and `from_angle+shortest_angle` are both inside the limits, and `fmod(to_angle+shortest_angle, 2*M_PI)` equals `fmod(to_angle, 2*M_PI)`.
        On the other hand, `shortest_angular_distance_with_large_limits(10.5*M_PI, 0, -2*M_PI, 2*M_PI)` will return false, since `from_angle` is not in the valid range.
        Finally, note that the call `shortest_angular_distance_with_large_limits(0, 10.5*M_PI, -2*M_PI, 0.1*M_PI)` will also return `true`.
        However, `shortest_angle` in this case will be `-1.5*M_PI`.

        \return valid_flag, shortest_angle - valid_flag will be true if `left_limit < right_limit` and if "from_angle" and "from_angle+shortest_angle" positions are within the valid interval, false otherwise.
        \param left_limit - left limit of valid interval, must be smaller than right_limit.
        \param right_limit - right limit of valid interval, must be greater than left_limit.
    """
    # Shortest steps in the two directions
    delta = shortest_angular_distance(from_angle, to_angle)
    delta_2pi = two_pi_complement(delta)

    # "sort" distances so that delta is shorter than delta_2pi
    if fabs(delta) > fabs(delta_2pi):
        delta, delta_2pi = delta_2pi, delta

    if left_limit > right_limit:
        # If limits are something like [PI/2 , -PI/2] it actually means that we
        # want rotations to be in the interval [-PI,PI/2] U [PI/2,PI], ie, the
        # half unit circle not containing the 0. This is already gracefully
        # handled by shortest_angular_distance_with_limits, and therefore this
        # function should not be called at all. However, if one has limits that
        # are larger than PI, the same rationale behind shortest_angular_distance_with_limits
        # does not hold, ie, M_PI+x should not be directly equal to -M_PI+x.
        # In this case, the correct way of getting the shortest solution is to
        # properly set the limits, eg, by saying that the interval is either
        # [PI/2, 3*PI/2] or [-3*M_PI/2, -M_PI/2]. For this reason, here we
        # return false by default.
        return False, delta
    

    # Check in which direction we should turn (clockwise or counter-clockwise).

    # start by trying with the shortest angle (delta).
    to2 = from_angle + delta
    if left_limit <= to2 and to2 <= right_limit:
        # we can move in this direction: return success if the "from" angle is inside limits
        valid_flag = left_limit <= from_angle and from_angle <= right_limit
        return valid_flag, delta

    # delta is not ok, try to move in the other direction (using its complement)
    to2 = from_angle + delta_2pi
    if left_limit <= to2 and to2 <= right_limit:
        # we can move in this direction: return success if the "from" angle is inside limits
        valid_flag = left_limit <= from_angle and from_angle <= right_limit
        return valid_flag, delta_2pi

    # nothing works: we always go outside limits
    return False, delta
