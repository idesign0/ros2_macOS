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

#ifndef GZ_PLUGIN_EXAMPLES_PLUGINS_ROBOT_HH_
#define GZ_PLUGIN_EXAMPLES_PLUGINS_ROBOT_HH_

#include <utility>

#include <gz/math/Vector2.hh>
#include <gz/math/Vector3.hh>

#include <gz/plugin/Register.hh>

namespace gz
{
  namespace plugin
  {
    namespace examples
    {
      /////////////////////////////////////////////////
      /// \brief Interface for driving.
      class Drive
      {
        /// \brief Indicate the frequency of this robot's control loop in Hz.
        /// The inverse of this value will be the size of the time step in the
        /// simulation.
        /// \return Control loop frequency in Hz.
        public: virtual double Frequency() const = 0;

        /// \brief Indicate how fast the robot is driving in SE(2).
        /// \return The first two components are x/y velocity (m/s). The third
        /// component is the yaw velocity (rad/s about the z-axis). The x/y
        /// coordinates are always relative to the robot's frame.
        public: virtual gz::math::Vector3d Velocity() const = 0;

        /// \brief Virtual destructor
        public: virtual ~Drive() = default;
      };

      /////////////////////////////////////////////////
      /// \brief Interface for a proximity sensor.
      class ProximitySensor
      {
        /// \brief Return the maximum range of the proximity sensor.
        /// \return The maximum range of the proximity sensor, in meters.
        public: virtual double MaxRange() const = 0;

        /// \brief Receive a proximity reading (how far in front of the robot
        /// the nearest wall is).
        /// \param[in] _distance The distance (m) to the nearest wall in front
        /// of the robot. If the nearest distance is outside the max range, the
        /// value passed in will be infinity, which can be checked using the
        /// function std::isinf(~).
        public: virtual void ReadProximity(double _distance) = 0;

        /// \brief Virtual destructor
        public: virtual ~ProximitySensor() = default;
      };

      /////////////////////////////////////////////////
      /// \brief Interface for a GPS sensor.
      class GPSSensor
      {
        /// \brief Get a GPS reading.
        /// \param[in] _location The world-frame x/y location of the robot.
        public: virtual void ReadGPS(
          const gz::math::Vector2d &_location) = 0;

        /// \brief Virtual destructor
        public: virtual ~GPSSensor() = default;
      };

      /////////////////////////////////////////////////
      /// \brief Interface for a compass sensor.
      class Compass
      {
        /// \brief Get a compass reading.
        /// \param[in] _orientation The world-frame yaw of the robot.
        public: virtual void ReadCompass(double _orientation) = 0;

        /// \brief Virtual destructor
        public: virtual ~Compass() = default;
      };

      /////////////////////////////////////////////////
      /// \brief A pair of Vector2d points defines a wall.
      using Wall =
          std::pair<gz::math::Vector2d, gz::math::Vector2d>;

      /////////////////////////////////////////////////
      /// \brief The layout of an environment is defined by a set of walls.
      using Layout = std::vector<Wall>;

      /////////////////////////////////////////////////
      /// \brief Interface for robots that have a map database. Robot plugins
      /// with this interface will be given a layout of the map before the
      /// simulation begins.
      class MapDatabase
      {
        /// \brief Get a complete map reading.
        /// \param[in] _map The complete map of the environment.
        public: virtual void ReadMap(const Layout &_map) = 0;

        /// \brief Virtual destructor
        public: virtual ~MapDatabase() = default;
      };

      /////////////////////////////////////////////////
      /// \brief Interface for defining an environment.
      class Environment
      {
        /// \brief Get the layout of the environment.
        /// \return A set of pairs of points. Each pair represents a wall.
        public: virtual Layout GenerateLayout() const = 0;

        /// \brief Virtual destructor
        public: virtual ~Environment() = default;
      };
    }
  }
}

#endif
