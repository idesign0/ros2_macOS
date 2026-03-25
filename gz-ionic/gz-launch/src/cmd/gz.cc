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
#include <cstring>
#include <string>
#include <gz/common/Console.hh>

#include "gz/launch/config.hh"
#include "gz/launch/InstallationDirectories.hh"
#include "gz.hh"
#include "../Manager.hh"

//////////////////////////////////////////////////
extern "C" char *gzVersion()
{
  return strdup(GZ_LAUNCH_VERSION_FULL);
}

//////////////////////////////////////////////////
extern "C" const char *configPath()
{
  std::string configPath = gz::launch::getPluginInstallPath();
  return strdup(configPath.c_str());
}

//////////////////////////////////////////////////
extern "C" void cmdVerbosity(const int _verbosity)
{
  gz::common::Console::SetVerbosity(_verbosity);
}

//////////////////////////////////////////////////
extern "C" int run(const char *_config)
{
  gz::launch::Manager mgr;
  return mgr.RunConfig(_config);
}
