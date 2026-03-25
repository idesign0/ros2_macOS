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
#ifndef GZ_LAUNCH_GZ_HH_
#define GZ_LAUNCH_GZ_HH_

#include "gz/launch/Export.hh"

/// \brief External hook to read the library version.
/// \return C-string representing the version. Ex.: 0.1.2
extern "C" char *gzVersion();

/// \brief Set verbosity level
/// \param[in] _verbosity 0 to 4
extern "C" void cmdVerbosity(
    const int _verbosity);

/// \brief External hook to execute 'gz fuel list -t model' from the command
/// line.
/// \param[in] _config Config file to run.
/// \return 1 if successful, 0 if not.
extern "C" int run(const char *_config);

#endif
