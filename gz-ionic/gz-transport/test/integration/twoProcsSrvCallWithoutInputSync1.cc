/*
 * Copyright (C) 2016 Open Source Robotics Foundation
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
#include <gz/msgs/int32.pb.h>

#include <chrono>
#include <cstdlib>
#include <string>

#include "gz/transport/Node.hh"

#include <gz/utils/Environment.hh>
#include <gz/utils/Subprocess.hh>

#include "gtest/gtest.h"

#include "test_config.hh"
#include "test_utils.hh"

using namespace gz;

static std::string g_partition; // NOLINT(*)
static std::string g_topic = "/foo"; // NOLINT(*)

//////////////////////////////////////////////////
/// \brief This test spawns a service that doesn't accept input parameters. The
/// synchronous requester uses a wrong service's name. The test should verify
/// that the service call does not succeed and the elapsed time was close to
/// the timeout.
TEST(twoProcSrvCallWithoutInputSync1, SrvTwoProcs)
{
  auto pi = gz::utils::Subprocess(
      {test_executables::kTwoProcsSrvCallWithoutInputReplier, g_partition});

  int64_t timeout = 500;
  msgs::Int32 rep;
  bool result;

  transport::Node node;

  // Make sure that the address of the service call provider is known.
  std::this_thread::sleep_for(std::chrono::milliseconds(3000));
  ASSERT_TRUE(node.Request(g_topic, static_cast<unsigned int>(timeout), rep,
    result));
  EXPECT_TRUE(result);

  auto t1 = std::chrono::steady_clock::now();
  EXPECT_FALSE(node.Request("unknown_service",
    static_cast<unsigned int>(timeout), rep, result));
  auto t2 = std::chrono::steady_clock::now();

  int64_t elapsed =
    std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

  // Check if the elapsed time was close to the timeout.
  auto diff = std::max(elapsed, timeout) - std::min(elapsed, timeout);
  EXPECT_LT(diff, 200);
}

//////////////////////////////////////////////////
int main(int argc, char **argv)
{
  // Get a random partition name.
  g_partition = testing::getRandomNumber();

  // Set the partition name for this process.
  gz::utils::setenv("GZ_PARTITION", g_partition);

  // Enable verbose mode.
  gz::utils::setenv("GZ_VERBOSE", "1");

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
