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

#include <gz/transport/log/Export.hh>

extern "C"
{
  enum {
    SUCCESS             = 0,
    FAILED_TO_OPEN      = 1,
    BAD_REGEX           = 2,
    FAILED_TO_ADVERTISE = 3,
    FAILED_TO_SUBSCRIBE = 4,
    INVALID_VERSION     = 5,
    INVALID_REMAP       = 6,
  };

  /// \brief Sets verbosity of library
  /// \param[in] _level [0-4] Verbosity level
  int GZ_TRANSPORT_LOG_VISIBLE verbosity(int _level);

  /// \brief Record topics whose name matches the given pattern
  /// \param[in] _file Path to the log file to record
  /// \param[in] _pattern ECMAScript regular expression to match against topics
  int GZ_TRANSPORT_LOG_VISIBLE recordTopics(
    const char *_file,
    const char *_pattern);

  /// \brief Playback topics whose name matches the given pattern
  /// \param[in] _file Path to the log file to playback
  /// \param[in] _pattern ECMAScript regular expression to match against topics
  /// \param[in] _wait_ms How long to wait before the publications begin after
  /// advertising the topics that will be played back (milliseconds)
  /// \param[in] _fast Set to > 0 to disable wait between messages.
  int GZ_TRANSPORT_LOG_VISIBLE playbackTopics(
    const char *_file,
    const char *_pattern,
    const int _wait_ms,
    const char *_remap,
    int _fast);
}
