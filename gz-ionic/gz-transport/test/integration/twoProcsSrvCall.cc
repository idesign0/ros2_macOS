/*
 * Copyright (C) 2014 Open Source Robotics Foundation
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
#include <gz/msgs/vector3d.pb.h>

#include <chrono>
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

#include "gz/transport/Node.hh"
#include "gz/transport/TopicUtils.hh"

#include <gz/utils/Environment.hh>
#include "gz/utils/Subprocess.hh"

#include "gtest/gtest.h"
#include "test_config.hh"
#include "test_utils.hh"

using namespace gz;

static bool responseExecuted;
static bool wrongResponseExecuted;

static std::string g_topic = "/foo"; // NOLINT(*)
static int data = 5;
static int counter = 0;

//////////////////////////////////////////////////
class twoProcSrvCall: public testing::Test {
 protected:
  void SetUp() override {
    gz::utils::env("GZ_PARTITION", this->prevPartition);

    // Get a random partition name.
    this->partition = testing::getRandomNumber();

    // Set the partition name for this process.
    gz::utils::setenv("GZ_PARTITION", this->partition);

    this->pi = std::make_unique<gz::utils::Subprocess>(
      std::vector<std::string>({
        test_executables::kTwoProcsSrvCallReplier, this->partition}));
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
  responseExecuted = false;
  wrongResponseExecuted = false;
  counter = 0;
}

//////////////////////////////////////////////////
/// \brief Service call response callback.
void response(const msgs::Int32 &_rep, const bool _result)
{
  EXPECT_EQ(_rep.data(), data);
  EXPECT_TRUE(_result);

  responseExecuted = true;
  ++counter;
}

//////////////////////////////////////////////////
/// \brief Service call response callback.
void wrongResponse(const msgs::Vector3d &/*_rep*/, bool /*_result*/)
{
  wrongResponseExecuted = true;
}

//////////////////////////////////////////////////
/// \brief Two different nodes running in two different processes. One node
/// advertises a service and the other requests a few service calls.
TEST_F(twoProcSrvCall, SrvTwoProcs)
{
  reset();

  msgs::Int32 req;
  req.set_data(data);

  transport::Node node;
  EXPECT_TRUE(node.Request(g_topic, req, response));

  int i = 0;
  while (i < 300 && !responseExecuted)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ++i;
  }

  // Check that the service call response was executed.
  EXPECT_TRUE(responseExecuted);
  EXPECT_EQ(counter, 1);

  // Make another request.
  reset();

  EXPECT_TRUE(node.Request(g_topic, req, response));

  i = 0;
  while (i < 300 && !responseExecuted)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ++i;
  }

  // Check that the service call response was executed.
  EXPECT_TRUE(responseExecuted);
  EXPECT_EQ(counter, 1);

  reset();
}

//////////////////////////////////////////////////
/// \brief This test spawns a service responser and a service requester. The
/// requester uses a wrong type for the request argument. The test should verify
/// that the service call does not succeed.
TEST_F(twoProcSrvCall, SrvRequestWrongReq)
{
  msgs::Vector3d wrongReq;
  msgs::Int32 rep;
  bool result;
  unsigned int timeout = 1000;

  wrongReq.set_x(1);
  wrongReq.set_y(2);
  wrongReq.set_z(3);

  reset();

  transport::Node node;

  // Request an asynchronous service call with wrong type in the request.
  EXPECT_TRUE(node.Request(g_topic, wrongReq, response));
  std::this_thread::sleep_for(std::chrono::milliseconds(300));
  EXPECT_FALSE(responseExecuted);

  // Request a synchronous service call with wrong type in the request.
  EXPECT_FALSE(node.Request(g_topic, wrongReq, timeout, rep, result));

  reset();
}

//////////////////////////////////////////////////
/// \brief This test spawns a service responser and a service requester. The
/// requester uses a wrong type for the response argument. The test should
/// verify that the service call does not succeed.
TEST_F(twoProcSrvCall, SrvRequestWrongRep)
{
  msgs::Int32 req;
  msgs::Vector3d wrongRep;
  bool result;
  unsigned int timeout = 1000;

  req.set_data(data);

  reset();

  transport::Node node;

  // Request an asynchronous service call with wrong type in the response.
  EXPECT_TRUE(node.Request(g_topic, req, wrongResponse));
  std::this_thread::sleep_for(std::chrono::milliseconds(300));
  EXPECT_FALSE(wrongResponseExecuted);

  // Request a synchronous service call with wrong type in the response.
  EXPECT_FALSE(node.Request(g_topic, req, timeout, wrongRep, result));

  reset();
}

//////////////////////////////////////////////////
/// \brief This test spawns a service responser and two service requesters. The
/// service requesters use incorrect types in some of the requests. The test
/// should verify that a response is received only when the appropriate types
/// are used.
TEST_F(twoProcSrvCall, SrvTwoRequestsOneWrong)
{
  msgs::Int32 req;
  msgs::Int32 goodRep;
  msgs::Vector3d badRep;
  bool result;
  unsigned int timeout = 2000;

  req.set_data(data);

  reset();

  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  transport::Node node;

  // Request service calls with wrong types in the response.
  EXPECT_FALSE(node.Request(g_topic, req, timeout, badRep, result));
  EXPECT_TRUE(node.Request(g_topic, req, wrongResponse));
  std::this_thread::sleep_for(std::chrono::milliseconds(300));
  EXPECT_FALSE(wrongResponseExecuted);

  reset();

  // Valid service requests.
  EXPECT_TRUE(node.Request(g_topic, req, timeout, goodRep, result));
  EXPECT_TRUE(node.Request(g_topic, req, response));
  std::this_thread::sleep_for(std::chrono::milliseconds(300));
  EXPECT_TRUE(responseExecuted);

  reset();
}

//////////////////////////////////////////////////
/// \brief This test spawns two nodes on different processes. One of the nodes
/// advertises a service and the other uses ServiceList() for getting the list
/// of available services.
TEST_F(twoProcSrvCall, ServiceList)
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

  EXPECT_LE(elapsed2, elapsed1) << "Elapsed2[" << elapsed2.count()
    << "] Elapsed1[" << elapsed1.count() << "]";

  reset();
}

//////////////////////////////////////////////////
/// \brief This test spawns two nodes on different processes. One of the nodes
/// advertises a service and the other uses ServiceInfo() for getting
/// information about the service.
TEST_F(twoProcSrvCall, ServiceInfo)
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
  EXPECT_EQ(publishers.front().RepTypeName(), "gz.msgs.Int32");

  reset();
}
