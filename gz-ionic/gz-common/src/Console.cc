/*
 * Copyright (C) 2016 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <spdlog/spdlog.h>
#ifdef _WIN32
#include <Windows.h>
#endif

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>

#include <gz/common/config.hh>
#include <gz/common/Console.hh>
#include <gz/common/Util.hh>
#include <gz/utils/log/Logger.hh>
#include <gz/utils/NeverDestroyed.hh>

using namespace gz;
using namespace common;

/////////////////////////////////////////////////
LogMessage::LogMessage(const char *_file, int _line,
  spdlog::level::level_enum _logLevel)
  : severity(_logLevel),
    sourceLocation(_file, _line, "")
{
  // Use default initialization if needed.
  static std::once_flag flag;
  std::call_once(flag, []{
    if (!Console::initialized)
      Console::Init(".gz", "auto_default.log");
  });
}

/////////////////////////////////////////////////
LogMessage::~LogMessage()
{
  gz::common::Console::Root().RawLogger().log(
    this->sourceLocation, this->severity,
    gz::common::Console::Prefix() + this->ss.str());
}

/////////////////////////////////////////////////
std::ostream &LogMessage::stream()
{
  return this->ss;
}

bool Console::initialized = false;
int Console::verbosity = 1;
std::string Console::customPrefix = ""; // NOLINT(*)

/////////////////////////////////////////////////
Console::Console(const std::string &_loggerName)
  : gz::utils::log::Logger(_loggerName)
{
}

/////////////////////////////////////////////////
Console &Console::Root()
{
  static gz::utils::NeverDestroyed<Console> root{"gz"};
  return root.Access();
}

/////////////////////////////////////////////////
bool Console::Init(const std::string &_directory, const std::string &_filename)
{
  std::string logPath;

  if (_directory.empty() ||
#ifndef _WIN32
    _directory[0] != '/'
#else
    _directory.length() < 2 || _directory[1] != ':'
#endif
    )
  {
    if (!env(GZ_HOMEDIR, logPath))
    {
      // Use stderr here to prevent infinite recursion
      // trying to get the log initialized
      std::cerr << "Missing HOME environment variable."
        << "No log file will be generated." << std::endl;
      return false;
    }
    logPath = joinPaths(logPath, _directory);
  }
  else
  {
    logPath = _directory;
  }

  logPath = joinPaths(logPath, _filename);

  Console::Root().SetLogDestination(logPath.c_str());
  Console::initialized = true;

  return true;
}

/////////////////////////////////////////////////
void Console::Close()
{
  // Detach the current file sink.
  Console::Root().SetLogDestination(std::string());
}

/////////////////////////////////////////////////
std::string Console::Directory()
{
  std::filesystem::path path = gz::common::Console::Root().LogDestination();
  return path.parent_path().string();
}

//////////////////////////////////////////////////
void Console::SetVerbosity(const int _level)
{
  if (_level < 0)
  {
    Console::Root().RawLogger().log(spdlog::level::err,
      "Negative verbosity level. Ignoring it");
    return;
  }

  switch (_level)
  {
  case 0:
    gz::common::Console::Root().SetConsoleSinkLevel(spdlog::level::critical);
    break;
  case 1:
    gz::common::Console::Root().SetConsoleSinkLevel(spdlog::level::err);
    break;
  case 2:
    gz::common::Console::Root().SetConsoleSinkLevel(spdlog::level::warn);
    break;
  case 3:
    gz::common::Console::Root().SetConsoleSinkLevel(spdlog::level::info);
    break;
  case 4:
    gz::common::Console::Root().SetConsoleSinkLevel(spdlog::level::debug);
    break;
  case 5:
    gz::common::Console::Root().SetConsoleSinkLevel(spdlog::level::trace);
    break;
  default:
    gz::common::Console::Root().SetConsoleSinkLevel(spdlog::level::trace);
  }

  verbosity = std::min(5, _level);
}

//////////////////////////////////////////////////
int Console::Verbosity()
{
  return verbosity;
}

//////////////////////////////////////////////////
void Console::SetPrefix(const std::string &_prefix)
{
  customPrefix = _prefix;
}

//////////////////////////////////////////////////
std::string Console::Prefix()
{
  return customPrefix;
}
