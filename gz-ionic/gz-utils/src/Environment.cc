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

#include <gz/utils/Environment.hh>

#include <algorithm>
#include <cstdlib>
#include <string>
#include <utility>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#include <processenv.h>
#endif

#ifndef _WIN32
extern char ** environ;
#endif

namespace gz
{
namespace utils
{
inline namespace GZ_UTILS_VERSION_NAMESPACE {

/////////////////////////////////////////////////
bool env(const std::string &_name, std::string &_value, bool _allowEmpty)
{
#ifdef _WIN32
  size_t requiredSize;
  getenv_s(&requiredSize, NULL, 0, _name.c_str());
  if (requiredSize == 0)
  {
    return false;
  }

  char* env_value = new char[requiredSize];
  getenv_s(&requiredSize, env_value, requiredSize, _name.c_str());
  _value = std::string(env_value);
  delete[] env_value;
  return true;
#else
  bool valid = false;
  std::string v;
  const char *cvar = std::getenv(_name.c_str());
  if (cvar != nullptr)
  {
    v = cvar;
    valid = true;

    if (v[0] == '\0' && !_allowEmpty)
    {
      valid = false;
    }
  }
  if (valid)
  {
    _value = v;
    return true;
  }
  return false;
#endif
}

/////////////////////////////////////////////////
bool setenv(const std::string &_name, const std::string &_value)
{
#ifdef _WIN32
  if (0 != _putenv_s(_name.c_str(), _value.c_str()))
  {
    return false;
  }
#else
  if (0 != ::setenv(_name.c_str(), _value.c_str(), true))
  {
    return false;
  }
#endif
  return true;
}

/////////////////////////////////////////////////
bool unsetenv(const std::string &_name)
{
#ifdef _WIN32
  if (0 != _putenv_s(_name.c_str(), ""))
  {
    return false;
  }
#else
  if (0 != ::unsetenv(_name.c_str()))
  {
    return false;
  }
#endif
  return true;
}

/////////////////////////////////////////////////
bool clearenv()
{
  bool success = true;
#if __linux__
  if (0 != ::clearenv())
  {
    success = false;
  }
#else
  // Windows and macOS don't have clearenv
  // so iterate and clear one-by-one
  for (const auto &[key, value] : env())
  {
    success &= unsetenv(key);
  }
#endif
  return success;
}

/////////////////////////////////////////////////
EnvironmentMap env()
{
  EnvironmentMap ret;

  char **currentEnv = nullptr;
#ifdef _WIN32
  currentEnv = *__p__environ();
#else
  currentEnv = environ;
#endif
  // In the case that clearenv() was just called
  // currentEnv will be nullptr
  if (currentEnv == nullptr)
    return {};

  std::vector<std::string> envStrings;
  for (; *currentEnv; ++currentEnv)
  {
    envStrings.emplace_back(*currentEnv);
  }
  return envStringsToMap(envStrings);
}

/////////////////////////////////////////////////
bool setenv(const EnvironmentMap &_vars)
{
  bool success = true;
  for (const auto &[key, value] : _vars)
  {
    success &= setenv(key, value);
  }
  return success;
}

/////////////////////////////////////////////////
EnvironmentMap envStringsToMap(const EnvironmentStrings &_envStrings)
{
  EnvironmentMap ret;
  for (const auto &pair : _envStrings)
  {
    auto eqPos = pair.find('=');
    if (eqPos != std::string::npos)
    {
      ret.emplace(pair.substr(0, eqPos), pair.substr(eqPos + 1));
    }
  }
  return ret;
}

/////////////////////////////////////////////////
EnvironmentStrings envMapToStrings(const EnvironmentMap &_envMap)
{
  EnvironmentStrings ret;
  auto sorted = std::vector<std::pair<std::string, std::string>>(
    _envMap.begin(), _envMap.end());
  std::sort(sorted.begin(), sorted.end());
  for (auto [key, value] : sorted)
  {
    key += '=';
    key += value;
    ret.push_back(key);
  }
  return ret;
}

/////////////////////////////////////////////////
std::string printenv()
{
  std::string ret;
  for (const auto &entry : envMapToStrings(env()))
  {
    ret.append(entry);
    ret.append("\n");
  }
  return ret;
}
}  // namespace GZ_UTILS_VERSION_NAMESPACE
}  // namespace utils
}  // namespace gz
