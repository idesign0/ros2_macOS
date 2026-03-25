/*
 * Copyright (C) 2021 Open Source Robotics Foundation
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
#include <gz/common/Console.hh>
#include <gz/common/Filesystem.hh>

#include "gz/fuel_tools/Helpers.hh"

namespace gz::fuel_tools
{
//////////////////////////////////////////////////
std::string sanitizeAuthority(const std::string &_uriAuthority)
{
  // Take an authority of the form userinfo@host:port and turn it into a valid
  // subset of characters for a path.
  auto output = gz::common::replaceAll(_uriAuthority, ":", "%3A");
  output = gz::common::replaceAll(output, "@", "%40");
  return output;
}

//////////////////////////////////////////////////
std::string uriToPath(const common::URI &_uri)
{
  auto path = _uri.Path().Str();
  if (_uri.Path().IsAbsolute())
  {
    path = path.substr(1);
  }
  common::changeFromUnixPath(path);

  if (!_uri.Authority())
  {
    return path;
  }

  auto authority = _uri.Authority()->Str();
  if (authority.find("//") == 0)
  {
    authority = authority.substr(2);
  }

  authority = sanitizeAuthority(authority);
  if (authority.empty())
  {
    return path;
  }

  if (path.empty())
  {
    return authority;
  }

  return common::joinPaths(authority, path);
}
}  // namespace gz::fuel_tools
