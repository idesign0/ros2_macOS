/*
 * Copyright (C) 2016 Open Source Robotics Foundation
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

#include <cstdlib>
#include <string>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "gz/transport/Helpers.hh"

namespace gz::transport
{
  inline namespace GZ_TRANSPORT_VERSION_NAMESPACE {
  //////////////////////////////////////////////////
  bool env(const std::string &_name, std::string &_value)
  {
    char *v;
#ifdef _MSC_VER
    size_t sz = 0;
    _dupenv_s(&v, &sz, _name.c_str());
#else
    v = std::getenv(_name.c_str());
#endif
    if (v)
    {
      _value = v;
      return true;
    }
    return false;
  }

  //////////////////////////////////////////////////
  std::vector<std::string> split(const std::string &_orig, char _delim)
  {
    std::vector<std::string> pieces;
    size_t pos1 = 0;
    size_t pos2 = _orig.find(_delim);
    while (pos2 != std::string::npos)
    {
      pieces.push_back(_orig.substr(pos1, pos2-pos1));
      pos1 = pos2 + 1;
      pos2 = _orig.find(_delim, pos2 + 1);
    }
    pieces.push_back(_orig.substr(pos1, _orig.size()-pos1));
    return pieces;
  }

  //////////////////////////////////////////////////
  unsigned int getProcessId()
  {
#ifdef _WIN32
    return ::GetCurrentProcessId();
#else
    return ::getpid();
#endif
  }
  }  // namespace GZ_TRANSPORT_VERSION_NAMESPACE
}  // namespace gz::transport
