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

#ifndef GZ_UTILS_ENVIRONMENT_HH_
#define GZ_UTILS_ENVIRONMENT_HH_

#include <gz/utils/config.hh>
#include <gz/utils/Export.hh>

#include <string>
#include <unordered_map>
#include <vector>

namespace gz
{
namespace utils
{
inline namespace GZ_UTILS_VERSION_NAMESPACE {

/// \brief Find the environment variable '_name' and return its value.
///
/// Note: This function is not thread-safe and should not be called
/// concurrently with `setenv` or `unsetenv`
///
/// \param[in] _name Name of the environment variable.
/// \param[out] _value Value if the variable was found.
/// \param[in] _allowEmpty Allow set-but-empty variables.
///           (Unsupported on Windows)
/// \return True if the variable was found or false otherwise.
bool GZ_UTILS_VISIBLE env(
    const std::string &_name, std::string &_value,
    bool _allowEmpty = false);

/// \brief Set the environment variable '_name'.
///
/// Note: On Windows setting an empty string (_value=="")
/// is the equivalent of unsetting the variable.
//
/// Note: This function is not thread-safe and should not be called
/// concurrently with `env` or `unsetenv`
///
/// \param[in] _name Name of the environment variable.
/// \param[in] _value Value of the variable to be set.
/// \return True if the variable was set or false otherwise.
bool GZ_UTILS_VISIBLE setenv(
    const std::string &_name, const std::string &_value);

/// \brief Unset the environment variable '_name'.
///
/// Note: This function is not thread-safe and should not be called
/// concurrently with `env` or `setenv`
///
/// \param[in] _name Name of the environment variable.
/// \return True if the variable was unset or false otherwise.
bool GZ_UTILS_VISIBLE unsetenv(const std::string &_name);

/// \brief Unset all environment variables
///
/// Note: This function is not thread-safe and should not be called
/// concurrently with `env` or `setenv`
///
/// \return True if the environment was unset or false otherwise.
bool GZ_UTILS_VISIBLE clearenv();

/// \brief Type alias for a collection of environment variables
using EnvironmentMap = std::unordered_map<std::string, std::string>;

/// \brief Type alias for a collection of environment variables
/// Each entry is of the form KEY=VAL
using EnvironmentStrings = std::vector<std::string>;

/// \brief Convert a vector of environment variables to a map
///
/// \param[in] _envStrings Vector collection of environment variables
/// \return Mapped collection of environment variables.
EnvironmentMap GZ_UTILS_VISIBLE envStringsToMap(
  const EnvironmentStrings &_envStrings);

/// \brief Convert a map of environment variables to a vector
///
/// \param[in] _envMap Collection of mapped environment variables.
/// \return  Vector collection of environment variables.
EnvironmentStrings GZ_UTILS_VISIBLE envMapToStrings(
  const EnvironmentMap &_envMap);

/// \brief Retrieve all current environment variables
///
/// Note: This function is not thread-safe and should not be called
/// concurrently with `setenv` or `unsetenv`
///
/// \return A collection of current environment variables
EnvironmentMap GZ_UTILS_VISIBLE env();

/// \brief Set the environment variable '_name'.
///
/// Note: On Windows setting an empty string (_value=="")
/// is the equivalent of unsetting the variable.
//
/// Note: This function is not thread-safe and should not be called
/// concurrently with `env` or `unsetenv`
///
/// \param[in] _vars Collection of environment variables to set
/// \return True if all variables were set or false otherwise.
bool GZ_UTILS_VISIBLE setenv(const EnvironmentMap &_vars);

/// \brief Print the entire current environment to a string
///
/// This prints each variable in the form KEY=VALUE\n
///
/// Note: This function is not thread-safe and should not be called
/// concurrently with `setenv` or `unsetenv`
///
/// \return A string containing all environment variables
/// NOLINTNEXTLINE - This is incorrectly parsed as a global variable
std::string GZ_UTILS_VISIBLE printenv();
}  // namespace GZ_UTILS_VERSION_NAMESPACE
}  // namespace utils
}  // namespace gz

#endif  // GZ_UTILS_ENVIRONMENT_HH_
