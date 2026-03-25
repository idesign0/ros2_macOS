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

#include <chrono>
#include <iostream>

#include <gz/msgs/int32.pb.h>
#include <gz/transport/Node.hh>

#include <gz/utils/Environment.hh>

#include "ChirpParams.hh"

//////////////////////////////////////////////////
/// \brief chirp Create publishers to chirp out little messages for testing.
/// \param _topicNames A list of the topics that we want to chirp to
/// \param _chirps The number of chirps to set off
void chirp(const std::vector<std::string> &_topicNames,
           const int _chirps)
{
  std::cout << "Chirping ["<< _chirps << "] times on [" << _topicNames.size()
         << "] topics:\n";
  for (const std::string &name : _topicNames)
    std::cout << " -- " << name << "\n";

  gz::transport::Node node;

  std::vector<gz::transport::Node::Publisher> publishers;

  for (const std::string &topic : _topicNames)
  {
    publishers.push_back(node.Advertise<gz::msgs::Int32>(topic));
  }

  std::this_thread::sleep_for(
        std::chrono::milliseconds(
          gz::transport::log::test::DelayBeforePublishing_ms));

  gz::msgs::Int32 integer;
  integer.set_data(0);

  for (int c = 1; c <= _chirps; ++c)
  {
    integer.set_data(c);
    for (auto &pub : publishers)
    {
      // std::cout << "Chirping [" << c << "] on publisher [" << &pub << "]\n";
      pub.Publish(integer);
    }

    std::this_thread::sleep_for(
          std::chrono::milliseconds(
            gz::transport::log::test::DelayBetweenChirps_ms));
  }
}

//////////////////////////////////////////////////
int main(int argc, char **argv)
{
  // Argument list:
  // [0]: Name of current process
  // [1]: Partition name (used for setting the GZ_PARTITION env variable)
  // [2]: Number of times that the topics should chirp
  // [3]-[N]: A name for each topic that should chirp

  if (argc < 2)
  {
    std::cerr << "Missing partition name and number of chirps\n";
    return -1;
  }

  if (argc < 3)
  {
    std::cerr << "Missing number of chirps\n";
    return -2;
  }

  gz::utils::setenv("GZ_PARTITION", argv[1]);

  const int chirps = atoi(argv[2]);

  std::vector<std::string> topicNames;
  for (int t = 3; t < argc; ++t)
  {
    topicNames.emplace_back(argv[t]);
  }

  chirp(topicNames, chirps);
}
