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

#ifndef GZ_TRANSPORT_LOG_TEST_INTEGRATION_CHIRPPARAMS_HH_
#define GZ_TRANSPORT_LOG_TEST_INTEGRATION_CHIRPPARAMS_HH_

#include <string>
#include <vector>

#include <gz/msgs/int32.pb.h>
#include <gz/transport/Node.hh>
#include <gz/utils/Subprocess.hh>

namespace gz::transport::log::test
{
  /// \brief Parameter used to determine how long the topicChirp_aux
  /// program will wait between emitting message chirps from its topic.
  /// Value is in milliseconds.
  const int DelayBetweenChirps_ms = 1;

  /// \brief Parameter used to determine how long the topicChirp_aux
  /// program will wait (after it advertises) before it begins publishing
  /// its message chirps. Value is in milliseconds.
  const int DelayBeforePublishing_ms = 1000;

  /// \brief This is the message type that will be used by the chirping
  /// topics.
  using ChirpMsgType = gz::msgs::Int32;

  //////////////////////////////////////////////////
  /// \brief Similar to testing::forkAndRun(), except this function
  /// specifically calls the INTEGRATION_topicChirp_aux process and passes
  /// it arguments to determine how it should chirp out messages over its
  /// topics.
  /// \param[in] _topics A list of topic names to chirp on
  /// \param[in] _chirps The number of messages to chirp out. Each message
  /// will count up starting from the value 1 and ending with the value
  /// _chirps.
  /// \param[in] _paritionName Gz transport partition to use for the test
  /// \return A handle to the process. This can be used with
  /// testing::waitAndCleanupFork().
  gz::utils::Subprocess BeginChirps(
      const std::vector<std::string> &_topics,
      const int _chirps,
      const std::string &_partitionName);
}  // namespace gz::transport::log::test

#endif
