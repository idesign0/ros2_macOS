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
#include <gz/msgs/vector3d.pb.h>

#include <chrono>
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

#include "gz/transport/Node.hh"
#include "gz/transport/TopicUtils.hh"

#include <gz/utils/Environment.hh>
#include <gz/utils/Subprocess.hh>

#include "gtest/gtest.h"

#include "test_config.hh"
#include "test_utils.hh"

using namespace gz;

static bool g_responseExecuted;
static bool g_wrongResponseExecuted;

static std::string g_partition; // NOLINT(*)
static std::string g_topic = "/foo"; // NOLINT(*)
static int g_counter = 0;

//////////////////////////////////////////////////
class twoProcSrvCallWithoutOutput: public testing::Test {
 protected:
  void SetUp() override {
    gz::utils::env("GZ_PARTITION", this->prevPartition);

    // Get a random partition name.
    this->partition = testing::getRandomNumber();

    // Set the partition name for this process.
    gz::utils::setenv("GZ_PARTITION", this->partition);

    this->pi = std::make_unique<gz::utils::Subprocess>(
      std::vector<std::string>({
        test_executables::kTwoProcsSrvCallWithoutOutputReplier,
        this->partition}));
  }

  void TearDown() override {
    gz::utils::setenv("GZ_PARTITION", this->prevPartition);

    this->pi->Terminate();
    this->pi->Join();
  }

 private:
  std::string prevPartition;
  std::string partition;
  std::unique_ptr<gz::utils::Subprocess> pi;
};

//////////////////////////////////////////////////
/// \brief Initialize some global variables.
void reset()
{
  g_responseExecuted = false;
  g_wrongResponseExecuted = false;
  g_counter = 0;
}

//////////////////////////////////////////////////
/// \brief This test spawns a service that doesn't wait for output parameters.
/// The requester uses a wrong type for the request argument. The test should
/// verify that the service call does not succeed.
TEST_F(twoProcSrvCallWithoutOutput, SrvRequestWrongReq)
{
  msgs::Vector3d wrongReq;

  wrongReq.set_x(1);
  wrongReq.set_y(2);
  wrongReq.set_z(3);

  reset();

  transport::Node node;

  // Request an asynchronous service call with wrong type in the request.
  EXPECT_TRUE(node.Request(g_topic, wrongReq));
  std::this_thread::sleep_for(std::chrono::milliseconds(300));
  EXPECT_FALSE(g_responseExecuted);

  reset();
}

//////////////////////////////////////////////////
/// \brief This test spawns two nodes on different processes. One of the nodes
/// advertises a service without output and the other uses ServiceList() for
/// getting the list of available services.
TEST_F(twoProcSrvCallWithoutOutput, ServiceList)
{
  reset();

  transport::Node node;

  // We need some time for discovering the other node.
  std::this_thread::sleep_for(std::chrono::milliseconds(2500));

  std::vector<std::string> services;
  auto start1 = std::chrono::steady_clock::now();
  node.ServiceList(services);
  auto end1 = std::chrono::steady_clock::now();
  ASSERT_EQ(services.size(), 1u);
  EXPECT_EQ(services.at(0), g_topic);
  services.clear();

  // Time elapsed to get the first service list
  auto elapsed1 = end1 - start1;

  auto start2 = std::chrono::steady_clock::now();
  node.ServiceList(services);
  auto end2 = std::chrono::steady_clock::now();
  EXPECT_EQ(services.size(), 1u);
  EXPECT_EQ(services.at(0), g_topic);

  // The first ServiceList() call might block if the discovery is still
  // initializing (it may happen if we run this test alone).
  // However, the second call should never block.
  auto elapsed2 = end2 - start2;
  EXPECT_LT(std::chrono::duration_cast<std::chrono::milliseconds>
      (elapsed2).count(), 2);

  EXPECT_LE(elapsed2, elapsed1);

  reset();
}

//////////////////////////////////////////////////
/// \brief This test spawns two nodes on different processes. One of the nodes
/// advertises a service without output and the other uses ServiceInfo() for
/// getting information about the service.
TEST_F(twoProcSrvCallWithoutOutput, ServiceInfo)
{
  reset();

  transport::Node node;
  std::vector<transport::ServicePublisher> publishers;

  // We need some time for discovering the other node.
  std::this_thread::sleep_for(std::chrono::milliseconds(2500));

  EXPECT_FALSE(node.ServiceInfo("@", publishers));
  EXPECT_EQ(publishers.size(), 0u);

  EXPECT_FALSE(node.ServiceInfo("/bogus", publishers));
  EXPECT_EQ(publishers.size(), 0u);

  EXPECT_TRUE(node.ServiceInfo("/foo", publishers));
  EXPECT_EQ(publishers.size(), 1u);
  EXPECT_EQ(publishers.front().ReqTypeName(), "gz.msgs.Int32");

  reset();
}
