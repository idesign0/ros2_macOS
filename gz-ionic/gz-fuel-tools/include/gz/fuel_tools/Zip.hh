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
#ifndef GZ_FUEL_TOOLS_ZIP_HH_
#define GZ_FUEL_TOOLS_ZIP_HH_

#include <memory>
#include <string>
#include <vector>

#include "gz/fuel_tools/Export.hh"

namespace gz::fuel_tools
{
  /// \brief A helper class for making REST requests.
  class GZ_FUEL_TOOLS_VISIBLE Zip
  {
    /// \brief Compress a file or directory
    /// \param[in] _src Path to file or directory to compress
    /// \param[in] _dst Output compressed file path
    public: static bool Compress(const std::string &_src,
        const std::string &_dst);

    /// \brief Extract a compressed file
    /// \param[in] _src Path to compressed file
    /// \param[in] _dst Output extracted file path
    public: static bool Extract(const std::string &_src,
        const std::string &_dst);
  };
}  // namespace gz::fuel_tools

#endif  // GZ_FUEL_TOOLS_ZIP_HH_
