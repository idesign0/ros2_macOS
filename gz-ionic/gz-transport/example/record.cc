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

/// \brief Example of recording all gz transport topics.
/// This will record all topics and currently published to a file.
/// Launch the gz-transport publisher example so this example has
/// something to record.

//! [complete]
#include <cstdint>
#include <iostream>
#include <regex>

#include <gz/transport/Node.hh>
#include <gz/transport/log/Recorder.hh>

//////////////////////////////////////////////////
int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    std::cerr << "Usage: " << argv[0] << " OUTPUT.tlog\n";
    return -1;
  }

  gz::transport::log::Recorder recorder;

  // Record all topics
  const int64_t addTopicResult = recorder.AddTopic(std::regex(".*"));
  if (addTopicResult < 0)
  {
    std::cerr << "An error occurred when trying to add topics: "
              << addTopicResult << "\n";
    return -1;
  }

  // Begin recording, saving received messages to the given file
  const auto result = recorder.Start(argv[1]);
  if (gz::transport::log::RecorderError::SUCCESS != result)
  {
    std::cerr << "Failed to start recording: " << static_cast<int64_t>(result)
              << "\n";
    return -2;
  }

  std::cout << "Press Ctrl+C to finish recording.\n  Recording... "
            << std::endl;

  // Wait until the interrupt signal is sent.
  gz::transport::waitForShutdown();

  recorder.Stop();

  std::cout << "\nRecording finished!" << std::endl;

  return 0;
}
//! [complete]
