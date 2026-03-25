/*
 * Copyright (C) 2018 Open Source Robotics Foundation
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

#ifndef GZ_FUELTOOLS_GZ_HH_
#define GZ_FUELTOOLS_GZ_HH_

#include "gz/fuel_tools/Export.hh"

/// \brief External hook to read the library version.
/// \return C-string representing the version. Ex.: 0.1.2
extern "C" GZ_FUEL_TOOLS_VISIBLE char *gzVersion();

/// \brief Set verbosity level
/// \param[in] _verbosity 0 to 4
extern "C" GZ_FUEL_TOOLS_VISIBLE void cmdVerbosity(
    const char *_verbosity);

/// \brief External hook to execute 'gz fuel list -t model' from the command
/// line.
/// \param[in] _url Optional server URL.
/// \param[in] _owner Optional owner name
/// \param[in] _raw 'true' for machine readable output.
/// \param[in] _configFile Path to a YAML configuration file.
/// \return 1 if successful, 0 if not.
extern "C" GZ_FUEL_TOOLS_VISIBLE int listModels(
    const char *_url = nullptr, const char *_owner = "",
    const char *_raw = "false", const char *_configFile = nullptr);

/// \brief External hook to execute 'gz fuel list -t world' from the command
/// line.
/// \param[in] _url Optional server URL.
/// \param[in] _owner Optional owner name
/// \param[in] _raw 'true' for machine readable output.
/// \param[in] _configFile Path to a YAML configuration file.
/// \return 1 if successful, 0 if not.
extern "C" GZ_FUEL_TOOLS_VISIBLE int listWorlds(
    const char *_url = nullptr, const char *_owner = "",
    const char *_raw = "false", const char *_configFile = nullptr);

/// \brief External hook to execute 'gz fuel download -u URL' from the command
/// line.
/// \param[in] _url Resource URL.
/// \param[in] _configFile Path to a YAML configuration file.
/// \param[in] _header An HTTP header.
/// \param[in] _type Type of resource to download from collection
/// \param[in] _jobs Number of parallel jobs for downloading collections.
/// \return 1 if successful, 0 if not.
extern "C" GZ_FUEL_TOOLS_VISIBLE int downloadUrl(
    const char *_url = nullptr, const char *_configFile = nullptr,
    const char *_header = nullptr, const char *_type = nullptr, int _jobs = 1);

/// \brief External hook to execute 'gz fuel upload -m path' from the command
/// line.
///
/// Example usage, including a private access token which is required:
///
/// `gz fuel upload -m ~/my_model --header "Private-Token: <access_token>"`
///
/// \param[in] _path Resource path.
/// \param[in] _url Server URL.
/// \param[in] _header An HTTP header.
/// \param[in] _private "1" to make the resource private, "0" to make it
/// public.
/// \param[in] _owner Upload the resource to the provided owner, or nullptr
/// to upload to the account specified by the token in the header.
/// \return 1 if successful, 0 if not.
extern "C" GZ_FUEL_TOOLS_VISIBLE int upload(const char *_path,
    const char *_url, const char *_header = nullptr,
    const char *_private = nullptr, const char *_owner = nullptr);

/// \brief External hook to execute 'gz fuel delete [options]' from the command
/// line.
///
/// Example usage, including a private access token which is required:
///
/// `gz fuel delete -u https://fuel.gazebosim.org/1.0/openrobotics/models/Ambulance --header "Private-Token: <access_token>"` // NOLINT
///
/// \param[in] _url Resource URL.
/// \param[in] _header An HTTP header.
/// \return 1 if successful, 0 if not.
extern "C" GZ_FUEL_TOOLS_VISIBLE int deleteUrl(
    const char *_url, const char *_header = nullptr);

/// \brief External hook to execute 'gz fuel meta --config2pbtxt path'
/// from the command line.
/// \param[in] _path Resource path.
/// \return 1 if successful, 0 if not.
extern "C" GZ_FUEL_TOOLS_VISIBLE int config2Pbtxt(
    const char *_path = nullptr);


/// \brief External hook to execute 'gz fuel meta --pbtxt2config path'
/// from the command line.
/// \param[in] _path Resource path.
/// \return 1 if successful, 0 if not.
extern "C" GZ_FUEL_TOOLS_VISIBLE int pbtxt2Config(const char *_path);

/// \brief External hook to execute 'gz fuel edit [options]' from the command
/// line.
///
/// Example usage, including a private access token which is required:
///
/// `gz fuel edit -u https://fuel.gazebosim.org/1.0/openrobotics/models/Ambulance --header "Private-Token: <access_token>"` --public // NOLINT
///
/// \param[in] _url Resource URL.
/// \param[in] _header An HTTP header.
/// \param[in] _private "1" to make the resource private, "0" to make it
///            public
/// \param[in] _path Resource path.
/// \return 1 if successful, 0 if not.
extern "C" GZ_FUEL_TOOLS_VISIBLE int editUrl(
    const char *_url, const char *_header = nullptr,
    const char *_private = nullptr,
    const char *_path = nullptr);

/// \brief External hook to execute 'gz fuel update [options] from the command'
/// line
/// \param[in] _onlyModels "1" to only update models.
/// \param[in] _onlyWorlds "1" to only update worlds.
/// \param[in] _header An HTTP header.
/// \return 1 if successful, 0 if not.
extern "C" GZ_FUEL_TOOLS_VISIBLE int update(
    const char *_onlyModels = nullptr, const char *_onlyWorlds = nullptr,
    const char *_header = nullptr);

#endif
