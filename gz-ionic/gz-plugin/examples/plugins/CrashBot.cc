/*
 * Copyright (C) 2018 Open Source Robotics Foundation
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

#include "robot.hh"

using namespace gz::plugin::examples;

namespace CrashBotLib {

/// \brief A robot that likes to crash into walls
class CrashBot
    : public virtual Drive,
      public virtual ProximitySensor
{
  // Documentation inherited
  public: double Frequency() const override
  {
    return 100.0;
  }

  // Documentation inherited
  public: gz::math::Vector3d Velocity() const override
  {
    // If the proximity sensor detects a wall, drive straight forward,
    // aggressively into the wall
    if (detectedWall)
      return gz::math::Vector3d(1.0, 0.0, 0.0);

    // If we have not detected a wall, drive in a spiral, searching for a wall
    return gz::math::Vector3d(0.5, 0.0, 20.0*GZ_PI/180.0);
  }

  // Documentation inherited
  public: double MaxRange() const override
  {
    return 4.0;
  }

  // Documentation inherited
  public: void ReadProximity(const double _distance) override
  {
    detectedWall = !std::isinf(_distance);
  }

  /// \brief A flag to keep track of whether we've found a wall
  public: bool detectedWall;
};

GZ_ADD_PLUGIN(CrashBot, Drive, ProximitySensor)

}
