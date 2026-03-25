/*
 * Copyright (C) 2019 Open Source Robotics Foundation
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
#ifndef GZ_LAUNCH_GAZEBOSERVER_HH_
#define GZ_LAUNCH_GAZEBOSERVER_HH_

#include <memory>
#include <gz/plugin/Register.hh>
#include <gz/sim/Server.hh>
#include "gz/launch/Plugin.hh"

namespace gz
{
  namespace launch
  {
    /// \brief Runs the Gazebo server.
    ///
    /// # Example usage
    /// <!-- Run the gazebo server with a set of plugins -->
    /// <plugin name="gz::launch::SimServer"
    ///         filename="gz-launch-sim">
    ///   <!-- The SDF file to run -->
    ///   <world_file>diff_drive.sdf</world_file>
    ///
    ///   <!-- The physics system -->
    ///   <plugin entity_name="<%= worldName %>"
    ///           entity_type="world"
    ///           filename="gz-sim-physics-system"
    ///           name="gz::sim::systems::v0::Physics">
    ///   </plugin>
    ///
    ///   <!-- Specify any other Gazebo sim plugins here. -->
    /// </plugin>
    class SimServer : public gz::launch::Plugin
    {
      /// \brief Constructor.
      public: SimServer();

      /// \brief Destructor.
      public: virtual ~SimServer() = default;

      // Documentation inherited.
      public: virtual bool Load(
                  const tinyxml2::XMLElement *_elem) override final;

      /// \brief Private data pointer
      private: std::unique_ptr<sim::Server> server;
    };
  }
}

// Register the plugin
GZ_ADD_PLUGIN(gz::launch::SimServer, gz::launch::Plugin)

#endif
