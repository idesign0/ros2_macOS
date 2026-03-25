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

#ifndef GZ_FUEL_TOOLS_HELPERS_HH_
#define GZ_FUEL_TOOLS_HELPERS_HH_

#include <string>
#include <gz/common/URI.hh>
#include <gz/fuel_tools/Export.hh>

// Use safer functions on Windows
#ifdef _MSC_VER
  #define gz_strcat strcat_s
  #define gz_strcpy strcpy_s
  #define gz_sprintf sprintf_s
  #define gz_strdup _strdup
#else
  #define gz_strcat std::strcat
  #define gz_strcpy std::strcpy
  #define gz_sprintf std::sprintf
  #define gz_strdup strdup
#endif

namespace gz::fuel_tools
{
/// \brief Convert the authority portion of a URI to a string
/// suitable to be used as a path on disk for all platforms.
///
/// It encodes illegal characters on Windows and Linux filesystems with
/// their corresponding URL-encoded values.
///
/// This assumes an authority of the form: `username@host:port`
/// "@" is encoded as %40
/// ":" is encoded as %3A
///
/// \param[in] _uriAuthority the authority section of the URI to convert.
/// \return String suitable to use in file paths
GZ_FUEL_TOOLS_VISIBLE
std::string sanitizeAuthority(const std::string &_uriAuthority);

/// \brief Convert a URI to a string suitable to use as a path on disk.
/// It strips the scheme and authority's `//` prefix.
/// \param[in] _uri URI to convert.
/// \return String suitable to use in file paths
GZ_FUEL_TOOLS_VISIBLE
std::string uriToPath(const gz::common::URI &_uri);
}  // namespace gz::fuel_tools

#endif  // GZ_FUEL_TOOLS_HELPERS_HH_
