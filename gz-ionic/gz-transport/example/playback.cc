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

/// \brief Example of playing back all messages from a log.
/// This will create publishers for all topics in a file and publish them
/// with the same timing that they were received.
/// Launch the gz-transport subscriber example if the log was created
/// by recording the publisher example.

//! [complete]
#include <cstdint>
#include <iostream>
#include <regex>
#include <gz/transport/log/Playback.hh>

//////////////////////////////////////////////////
int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    std::cerr << "Usage: " << argv[0] << " INPUT.tlog\n";
    return -1;
  }

  gz::transport::log::Playback player(argv[1]);

  // Playback all topics
  const int64_t addTopicResult = player.AddTopic(std::regex(".*"));
  if (addTopicResult == 0)
  {
    std::cout << "No topics to play back\n";
    return 0;
  }
  else if (addTopicResult < 0)
  {
    std::cerr << "Failed to advertise topics: " << addTopicResult
              << "\n";
    return -1;
  }

  // Begin playback
  const auto handle = player.Start();
  if (!handle)
  {
    std::cerr << "Failed to start playback\n";
    return -2;
  }

  // Wait until the player stops on its own
  std::cout << "Playing all messages in the log file\n";
  handle->WaitUntilFinished();

  return 0;
}
//! [complete]
