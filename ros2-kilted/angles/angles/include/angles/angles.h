/*********************************************************************
* Software License Agreement (BSD License)
*
*  Copyright (c) 2008, Willow Garage, Inc.
*  All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*   * Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*   * Redistributions in binary form must reproduce the above
*     copyright notice, this list of conditions and the following
*     disclaimer in the documentation and/or other materials provided
*     with the distribution.
*   * Neither the name of the Willow Garage nor the names of its
*     contributors may be used to endorse or promote products derived
*     from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
*  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
*  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
*  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
*  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
*  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
*  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
*  POSSIBILITY OF SUCH DAMAGE.
*********************************************************************/

#ifndef GEOMETRY_ANGLES_UTILS_H
#define GEOMETRY_ANGLES_UTILS_H

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif

#include <algorithm>
#include <cmath>

namespace angles
{

  /*!
   * \brief Convert degrees to radians
   */

  static inline double from_degrees(double degrees)
  {
    return degrees * M_PI / 180.0;
  }

  /*!
   * \brief Convert radians to degrees
   */
  static inline double to_degrees(double radians)
  {
    return radians * 180.0 / M_PI;
  }


  /*!
   * \brief normalize_angle_positive
   *
   *        Normalizes the angle to be 0 to 2*M_PI
   *        It takes and returns radians.
   */
  static inline double normalize_angle_positive(double angle)
  {
    const double result = fmod(angle, 2.0*M_PI);
    if(result < 0) return result + 2.0*M_PI;
    return result;
  }


  /*!
   * \brief normalize
   *
   * Normalizes the angle to be -M_PI circle to +M_PI circle
   * It takes and returns radians.
   *
   */
  static inline double normalize_angle(double angle)
  {
    const double result = fmod(angle + M_PI, 2.0*M_PI);
    if(result <= 0.0) return result + M_PI;
    return result - M_PI;
  }


  /*!
   * \function
   * \brief shortest_angular_distance
   *
   * Given 2 angles, this returns the shortest angular
   * difference.  The inputs and ouputs are of course radians.
   *
   * The result
   * would always be -pi <= result <= pi.  Adding the result
   * to "from" will always get you an equivelent angle to "to".
   */

  static inline double shortest_angular_distance(double from, double to)
  {
    return normalize_angle(to-from);
  }

  /*!
   * \function
   *
   * \brief returns the angle in [-2*M_PI, 2*M_PI]  going the other way along the unit circle.
   * \param angle The angle to which you want to turn in the range [-2*M_PI, 2*M_PI]
   * E.g. two_pi_complement(-M_PI/4) returns 7_M_PI/4
   * two_pi_complement(M_PI/4) returns -7*M_PI/4
   *
   */
  static inline double two_pi_complement(double angle)
  {
    //check input conditions
    if (angle > 2*M_PI || angle < -2.0*M_PI)
      angle = fmod(angle, 2.0*M_PI);
    if(angle < 0)
      return (2*M_PI+angle);
    else if (angle > 0)
      return (-2*M_PI+angle);

    return(2*M_PI);
  }

  /*!
   * \function
   *
   * \brief This function is only intended for internal use and not intended for external use. If you do use it, read the documentation very carefully. Returns the min and max amount (in radians) that can be moved from "from" angle to "left_limit" and "right_limit".
   * \return returns false if "from" angle does not lie in the interval [left_limit,right_limit]
   * \param from - "from" angle - must lie in [-M_PI, M_PI)
   * \param left_limit - left limit of valid interval for angular position - must lie in [-M_PI, M_PI], left and right limits are specified on the unit circle w.r.t to a reference pointing inwards
   * \param right_limit - right limit of valid interval for angular position - must lie in [-M_PI, M_PI], left and right limits are specified on the unit circle w.r.t to a reference pointing inwards
   * \param result_min_delta - minimum (delta) angle (in radians) that can be moved from "from" position before hitting the joint stop
   * \param result_max_delta - maximum (delta) angle (in radians) that can be movedd from "from" position before hitting the joint stop
   */
  static bool find_min_max_delta(double from, double left_limit, double right_limit, double &result_min_delta, double &result_max_delta)
  {
    double delta[4];

    delta[0] = shortest_angular_distance(from,left_limit);
    delta[1] = shortest_angular_distance(from,right_limit);

    delta[2] = two_pi_complement(delta[0]);
    delta[3] = two_pi_complement(delta[1]);

    if(delta[0] == 0)
    {
      result_min_delta = delta[0];
      result_max_delta = std::max<double>(delta[1],delta[3]);
      return true;
    }

    if(delta[1] == 0)
    {
        result_max_delta = delta[1];
        result_min_delta = std::min<double>(delta[0],delta[2]);
        return true;
    }


    double delta_min = delta[0];
    double delta_min_2pi = delta[2];
    if(delta[2] < delta_min)
    {
      delta_min = delta[2];
      delta_min_2pi = delta[0];
    }

    double delta_max = delta[1];
    double delta_max_2pi = delta[3];
    if(delta[3] > delta_max)
    {
      delta_max = delta[3];
      delta_max_2pi = delta[1];
    }


    //    printf("%f %f %f %f\n",delta_min,delta_min_2pi,delta_max,delta_max_2pi);
    if((delta_min <= delta_max_2pi) || (delta_max >= delta_min_2pi))
    {
      result_min_delta = delta_max_2pi;
      result_max_delta = delta_min_2pi;
      if(left_limit == -M_PI && right_limit == M_PI)
        return true;
      else
        return false;
    }
    result_min_delta = delta_min;
    result_max_delta = delta_max;
    return true;
  }


  /*!
   * \function
   *
   * \brief Returns the delta from `from_angle` to `to_angle`, making sure it does not violate limits specified by `left_limit` and `right_limit`.
   * This function is similar to `shortest_angular_distance_with_limits()`, with the main difference that it accepts limits outside the `[-M_PI, M_PI]` range.
   * Even if this is quite uncommon, one could indeed consider revolute joints with large rotation limits, e.g., in the range `[-2*M_PI, 2*M_PI]`.
   *
   * In this case, a strict requirement is to have `left_limit` smaller than `right_limit`.
   * Note also that `from` must lie inside the valid range, while `to` does not need to.
   * In fact, this function will evaluate the shortest (valid) angle `shortest_angle` so that `from+shortest_angle` equals `to` up to an integer multiple of `2*M_PI`.
   * As an example, a call to `shortest_angular_distance_with_large_limits(0, 10.5*M_PI, -2*M_PI, 2*M_PI, shortest_angle)` will return `true`, with `shortest_angle=0.5*M_PI`.
   * This is because `from` and `from+shortest_angle` are both inside the limits, and `fmod(to+shortest_angle, 2*M_PI)` equals `fmod(to, 2*M_PI)`.
   * On the other hand, `shortest_angular_distance_with_large_limits(10.5*M_PI, 0, -2*M_PI, 2*M_PI, shortest_angle)` will return false, since `from` is not in the valid range.
   * Finally, note that the call `shortest_angular_distance_with_large_limits(0, 10.5*M_PI, -2*M_PI, 0.1*M_PI, shortest_angle)` will also return `true`.
   * However, `shortest_angle` in this case will be `-1.5*M_PI`.
   *
   * \return true if `left_limit < right_limit` and if "from" and "from+shortest_angle" positions are within the valid interval, false otherwise.
   * \param from - "from" angle.
   * \param to - "to" angle.
   * \param left_limit - left limit of valid interval, must be smaller than right_limit.
   * \param right_limit - right limit of valid interval, must be greater than left_limit.
   * \param shortest_angle - result of the shortest angle calculation.
   */
  static inline bool shortest_angular_distance_with_large_limits(double from, double to, double left_limit, double right_limit, double &shortest_angle)
  {
    // Shortest steps in the two directions
    double delta = shortest_angular_distance(from, to);
    double delta_2pi = two_pi_complement(delta);

    // "sort" distances so that delta is shorter than delta_2pi
    if(std::fabs(delta) > std::fabs(delta_2pi))
      std::swap(delta, delta_2pi);

    if(left_limit > right_limit) {
      // If limits are something like [PI/2 , -PI/2] it actually means that we
      // want rotations to be in the interval [-PI,PI/2] U [PI/2,PI], ie, the
      // half unit circle not containing the 0. This is already gracefully
      // handled by shortest_angular_distance_with_limits, and therefore this
      // function should not be called at all. However, if one has limits that
      // are larger than PI, the same rationale behind shortest_angular_distance_with_limits
      // does not hold, ie, M_PI+x should not be directly equal to -M_PI+x.
      // In this case, the correct way of getting the shortest solution is to
      // properly set the limits, eg, by saying that the interval is either
      // [PI/2, 3*PI/2] or [-3*M_PI/2, -M_PI/2]. For this reason, here we
      // return false by default.
      shortest_angle = delta;
      return false;
    }

    // Check in which direction we should turn (clockwise or counter-clockwise).

    // start by trying with the shortest angle (delta).
    double to2 = from + delta;
    if(left_limit <= to2 && to2 <= right_limit) {
      // we can move in this direction: return success if the "from" angle is inside limits
      shortest_angle = delta;
      return left_limit <= from && from <= right_limit;
    }

    // delta is not ok, try to move in the other direction (using its complement)
    to2 = from + delta_2pi;
    if(left_limit <= to2 && to2 <= right_limit) {
      // we can move in this direction: return success if the "from" angle is inside limits
      shortest_angle = delta_2pi;
      return left_limit <= from && from <= right_limit;
    }

    // nothing works: we always go outside limits
    shortest_angle = delta; // at least give some "coherent" result
    return false;
  }


  /*!
   * \function
   *
   * \brief Returns the delta from "from_angle" to "to_angle" making sure it does not violate limits specified by left_limit and right_limit.
   * The valid interval of angular positions is [left_limit,right_limit]. E.g., [-0.25,0.25] is a 0.5 radians wide interval that contains 0.
   * But [0.25,-0.25] is a 2*M_PI-0.5 wide interval that contains M_PI (but not 0).
   * The value of shortest_angle is the angular difference between "from" and "to" that lies within the defined valid interval.
   * E.g. shortest_angular_distance_with_limits(-0.5,0.5,0.25,-0.25,ss) evaluates ss to 2*M_PI-1.0 and returns true while
   * shortest_angular_distance_with_limits(-0.5,0.5,-0.25,0.25,ss) returns false since -0.5 and 0.5 do not lie in the interval [-0.25,0.25]
   *
   * \return true if "from" and "to" positions are within the limit interval, false otherwise
   * \param from - "from" angle
   * \param to - "to" angle
   * \param left_limit - left limit of valid interval for angular position, left and right limits are specified on the unit circle w.r.t to a reference pointing inwards
   * \param right_limit - right limit of valid interval for angular position, left and right limits are specified on the unit circle w.r.t to a reference pointing inwards
   * \param shortest_angle - result of the shortest angle calculation
   */
  static inline bool shortest_angular_distance_with_limits(double from, double to, double left_limit, double right_limit, double &shortest_angle)
  {

    double min_delta = -2*M_PI;
    double max_delta = 2*M_PI;
    double min_delta_to = -2*M_PI;
    double max_delta_to = 2*M_PI;
    bool flag    = find_min_max_delta(from,left_limit,right_limit,min_delta,max_delta);
    double delta = shortest_angular_distance(from,to);
    double delta_mod_2pi  = two_pi_complement(delta);


    if(flag)//from position is within the limits
    {
      if(delta >= min_delta && delta <= max_delta)
      {
        shortest_angle = delta;
        return true;
      }
      else if(delta_mod_2pi >= min_delta && delta_mod_2pi <= max_delta)
      {
        shortest_angle = delta_mod_2pi;
        return true;
      }
      else //to position is outside the limits
      {
        find_min_max_delta(to,left_limit,right_limit,min_delta_to,max_delta_to);
          if(fabs(min_delta_to) < fabs(max_delta_to))
            shortest_angle = std::max<double>(delta,delta_mod_2pi);
          else if(fabs(min_delta_to) > fabs(max_delta_to))
            shortest_angle =  std::min<double>(delta,delta_mod_2pi);
          else
          {
            if (fabs(delta) < fabs(delta_mod_2pi))
              shortest_angle = delta;
            else
              shortest_angle = delta_mod_2pi;
          }
          return false;
      }
    }
    else // from position is outside the limits
    {
        find_min_max_delta(to,left_limit,right_limit,min_delta_to,max_delta_to);

          if(fabs(min_delta) < fabs(max_delta))
            shortest_angle = std::min<double>(delta,delta_mod_2pi);
          else if (fabs(min_delta) > fabs(max_delta))
            shortest_angle =  std::max<double>(delta,delta_mod_2pi);
          else
          {
            if (fabs(delta) < fabs(delta_mod_2pi))
              shortest_angle = delta;
            else
              shortest_angle = delta_mod_2pi;
          }
      return false;
    }

    shortest_angle = delta;
    return false;
  }
}

#endif
