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
#ifndef GZ_LAUNCH_PLUGIN_HH_
#define GZ_LAUNCH_PLUGIN_HH_

#include <tinyxml2.h>
#include <gz/plugin/SpecializedPluginPtr.hh>
#include <gz/launch/Export.hh>
#include <gz/launch/config.hh>

namespace gz
{
  namespace launch
  {
    // Inline bracket to help doxygen filtering.
    inline namespace GZ_LAUNCH_VERSION_NAMESPACE {
    /// \brief Base class for launch plugins.
    class Plugin
    {
      // Default destructor
      public: virtual ~Plugin() {}

      /// \brief Load function that each launch plugin must implement.
      /// \param[in] _elem Pointer to the XML for this plugin.
      /// \return True to keep the plugin alive. Return false to have the
      /// plugin unloaded immediately.
      public: virtual bool Load(const tinyxml2::XMLElement *_elem) = 0;
    };

    /// \brief Pointer to a launch plugin.
    using PluginPtr = gz::plugin::SpecializedPluginPtr<
      gz::launch::Plugin>;
    }
  }
}
#endif
