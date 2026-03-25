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

#include <cstring>
#include <iostream>
#include <string>
#include <unordered_set>

#include "gz/plugin/Loader.hh"
#include "gz/plugin/config.hh"
#include "gz.hh"

using Loader = gz::plugin::Loader;

//////////////////////////////////////////////////
extern "C" void cmdPluginInfo(
    const char *_plugin, int _verbose)
{
  if (!_plugin || std::string(_plugin).empty())
  {
    std::cerr << "Invalid plugin file name. Plugin name must not be empty.\n";
    return;
  }

  Loader pl;
  std::cout << "Loading plugin library file [" << _plugin << "]\n";

  // Print names of plugins exported by library file
  std::unordered_set<std::string> pluginNames = pl.LoadLib(_plugin);

  if (_verbose)
  {
    std::cout << pl.PrettyStr() << std::endl;
    return;
  }

  std::cout << "* Found " << pluginNames.size() << " plugin";
  if (pluginNames.size() != 1u)
  {
    std::cout << 's';
  }
  std::cout << " in library file:" << std::endl;

  for (const auto & pluginName : pluginNames)
  {
    std::cout << "  - " << pluginName << std::endl;
  }

  auto interfacesImplemented = pl.InterfacesImplemented();
  std::cout << "* Found " << interfacesImplemented.size() << " interface";
  if (interfacesImplemented.size() != 1u)
  {
    std::cout << 's';
  }
  std::cout << " in library file:" << std::endl;

  for (const auto & interfaceImplemented : interfacesImplemented)
  {
    std::cout << "  - " << interfaceImplemented << std::endl;
  }
}

//////////////////////////////////////////////////
extern "C" const char *gzVersion()
{
  return strdup(GZ_PLUGIN_VERSION_FULL);
}
