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

#include "LogCommandAPI.hh"

#include <csignal>
#include <iostream>
#include <regex>
#include <string>

#include <gz/transport/log/Export.hh>
#include <gz/transport/log/Playback.hh>
#include <gz/transport/log/Recorder.hh>
#include <gz/transport/Node.hh>
#include <gz/transport/NodeOptions.hh>
#include "../Console.hh"

using namespace gz;
transport::log::PlaybackHandlePtr g_playbackHandler;

//////////////////////////////////////////////////
int verbosity(int _level)
{
  if (_level < 0 || _level > 4)
  {
    std::cerr << "Invalid verbosity level\n";
    return INVALID_VERSION;
  }
  transport::log::__verbosity = _level;
  return SUCCESS;
}

//////////////////////////////////////////////////
int recordTopics(const char *_file, const char *_pattern)
{
  std::regex regexPattern;
  try
  {
    regexPattern = _pattern;
  }
  catch (const std::regex_error &e)
  {
    LERR("Regex pattern is invalid\n");
    return BAD_REGEX;
  }

  transport::log::Recorder recorder;

  if (recorder.AddTopic(regexPattern) < 0)
    return FAILED_TO_SUBSCRIBE;

  if (recorder.Start(_file) != transport::log::RecorderError::SUCCESS)
    return FAILED_TO_OPEN;

  // Wait until signaled (SIGINT, SIGTERM)
  transport::waitForShutdown();
  LDBG("Shutting down\n");
  recorder.Stop();

  return SUCCESS;
}

//////////////////////////////////////////////////
void playbackSignHandler(int) // NOLINT
{
  g_playbackHandler->Stop();
}

//////////////////////////////////////////////////
int playbackTopics(const char *_file, const char *_pattern, const int _wait_ms,
  const char *_remap, int _fast)
{
  std::regex regexPattern;
  try
  {
    regexPattern = _pattern;
  }
  catch (const std::regex_error &e)
  {
    LERR("Regex pattern is invalid\n");
    return BAD_REGEX;
  }

  // Parse remapping.
  transport::NodeOptions nodeOptions;
  std::string remap = std::string(_remap);
  if (!remap.empty())
  {
    // Sanity check: It should contain the := delimiter.
    auto delim = remap.find(":=");
    if (delim == std::string::npos)
      return INVALID_REMAP;

    std::string from = remap.substr(0, delim);
    std::string to = remap.substr(delim + 2, remap.size() - delim - 1);

    if (!nodeOptions.AddTopicRemap(from, to))
      return INVALID_REMAP;
  }

  transport::log::Playback player(_file, nodeOptions);
  if (!player.Valid())
    return FAILED_TO_OPEN;

  if (player.AddTopic(regexPattern) < 0)
    return FAILED_TO_ADVERTISE;

  std::this_thread::sleep_for(std::chrono::milliseconds(_wait_ms));

  std::signal(SIGINT, playbackSignHandler);
  std::signal(SIGTERM, playbackSignHandler);

  if (_fast)
    g_playbackHandler = player.Start(std::chrono::seconds(1), false);
  else
    g_playbackHandler = player.Start(std::chrono::seconds(1), true);

  if (!g_playbackHandler)
    return FAILED_TO_OPEN;

  // Wait until playback finishes
  g_playbackHandler->WaitUntilFinished();
  LDBG("Shutting down\n");
  return SUCCESS;
}
