/*
 * Copyright (C) 2017 Open Source Robotics Foundation
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

#ifndef GZ_FUEL_TOOLS_MODELPRIVATE_HH_
#define GZ_FUEL_TOOLS_MODELPRIVATE_HH_

#include <string>

#include "gz/fuel_tools/Helpers.hh"
#include "gz/fuel_tools/ModelIdentifier.hh"

#ifdef _WIN32
// Disable warning C4251 which is triggered by
// std::string
#pragma warning(push)
#pragma warning(disable: 4251)
#endif

namespace gz::fuel_tools
{
  /// \brief Private class, do not use
  class GZ_FUEL_TOOLS_VISIBLE ModelPrivate
  {
    /// \brief How this model is ID'd
    public: ModelIdentifier id;

    /// \brief Where this model is on disk
    public: std::string pathOnDisk;
  };
}
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#endif  // GZ_FUEL_TOOLS_MODELPRIVATE_HH_
