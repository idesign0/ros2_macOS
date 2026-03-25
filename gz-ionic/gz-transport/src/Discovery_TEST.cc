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
#include "gtest/gtest.h"

#include <chrono>
#include <cstdlib>
#include <memory>
#include <string>
#include <thread>

#include "gz/transport/AdvertiseOptions.hh"
#include "gz/transport/Discovery.hh"
#include "gz/transport/Publisher.hh"
#include "gz/transport/TransportTypes.hh"
#include "gz/transport/Uuid.hh"

#include "test_utils.hh"
#include "gz/utils/Environment.hh"
#include "gz/utils/ExtraTestMacros.hh"

using namespace gz;
using namespace transport;

// Global constants used for multiple tests.
static const int MaxIters = 100;
static const int Nap = 10;

// Global variables used for multiple tests.
static const int g_msgPort    = 11319;
static const int g_srvPort    = 11320;
static const std::string g_ip = "224.0.0.7"; // NOLINT(*)
static std::string g_topic = testing::getRandomNumber(); // NOLINT(*)
static std::string service = testing::getRandomNumber(); // NOLINT(*)
static std::string addr1   = "tcp://127.0.0.1:12345"; // NOLINT(*)
static std::string ctrl1   = "tcp://127.0.0.1:12346"; // NOLINT(*)
static std::string id1     = "identity1"; // NOLINT(*)
static std::string pUuid1  = Uuid().ToString(); // NOLINT(*)
static std::string nUuid1  = Uuid().ToString(); // NOLINT(*)
static std::string addr2   = "tcp://127.0.0.1:12347"; // NOLINT(*)
static std::string ctrl2   = "tcp://127.0.0.1:12348"; // NOLINT(*)
static std::string id2     = "identity2"; // NOLINT(*)
static std::string pUuid2  = Uuid().ToString(); // NOLINT(*)
static std::string nUuid2  = Uuid().ToString(); // NOLINT(*)
static bool connectionExecuted = false;
static bool disconnectionExecuted = false;
static int g_counter = 0;

/// \brief Helper class to access the protected member variables of Discovery
/// within the tests.
template<typename T> class DiscoveryDerived : public Discovery<T>
{
  // Documentation inherited.
  public: DiscoveryDerived(const std::string &_pUuid,
                           const std::string &_ip,
                           const int _port,
                           const bool _verbose = false)
    : Discovery<T>(_pUuid, _ip, _port, _verbose)
  {
  }

  /// \brief Check if this discovery node has some activity information about
  /// a given process.
  /// \param[in] _pUuid Process UUID that we want to check.
  /// \param[in] _expectedActivity If true, we expect activity on the process
  /// specified as a parameter. If false, we shouldn't have any activity stored.
  public: void TestActivity(const std::string &_pUuid,
                            const bool _expectedActivity) const
  {
    EXPECT_EQ(this->activity.find(_pUuid) !=
              this->activity.end(), _expectedActivity);
  };
};

//////////////////////////////////////////////////
/// \brief Initialize some global variables.
void reset()
{
  connectionExecuted = false;
  disconnectionExecuted = false;
  g_counter = 0;
}

//////////////////////////////////////////////////
/// \brief Helper function to wait some time until a callback is executed.
void waitForCallback(int _maxIters, int _sleepTimeIter, const bool &_var)
{
  int i = 0;
  while (i < _maxIters && !_var)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(_sleepTimeIter));
    ++i;
  }
}

//////////////////////////////////////////////////
/// \brief Function called each time a discovery update is received.
void onDiscoveryResponse(const MessagePublisher &_publisher)
{
  // This discovery event is not relevant for the test, ignore it.
  if (_publisher.NUuid() != nUuid1)
    return;

  EXPECT_EQ(_publisher.Addr(), addr1);
  EXPECT_EQ(_publisher.Ctrl(), ctrl1);
  EXPECT_EQ(_publisher.PUuid(), pUuid1);
  connectionExecuted = true;
}

//////////////////////////////////////////////////
/// \brief Function called each time a discovery update is received. This is
/// used in the case of multiple publishers.
void onDiscoveryResponseMultiple(const MessagePublisher &_publisher)
{
  // This discovery event is not relevant for the test, ignore it.
  if (_publisher.Topic() != g_topic)
    return;

  EXPECT_NE(_publisher.Addr(), "");
  EXPECT_NE(_publisher.Ctrl(), "");
  EXPECT_NE(_publisher.PUuid(), "");
  EXPECT_NE(_publisher.NUuid(), "");
  connectionExecuted = true;
  ++g_counter;
}

//////////////////////////////////////////////////
/// \brief Function called each time a discovery update is received.
void onDisconnection(const MessagePublisher &_publisher)
{
  // This discovery event is not relevant for the test, ignore it.
  if (_publisher.PUuid() != pUuid1)
    return;

  disconnectionExecuted = true;
}

//////////////////////////////////////////////////
/// \brief Test the setters, getters and basic functions.
TEST(DiscoveryTest, TestBasicAPI)
{
  unsigned int newSilenceInterval    = 100;
  unsigned int newActivityInterval   = 200;
  unsigned int newHeartbeatInterval  = 400;

  // Create a discovery node.
  Discovery<MessagePublisher> discovery(pUuid1, g_ip, g_msgPort);

  discovery.SetSilenceInterval(newSilenceInterval);
  discovery.SetActivityInterval(newActivityInterval);
  discovery.SetHeartbeatInterval(newHeartbeatInterval);

  EXPECT_EQ(discovery.SilenceInterval(), newSilenceInterval);
  EXPECT_EQ(discovery.ActivityInterval(), newActivityInterval);
  EXPECT_EQ(discovery.HeartbeatInterval(), newHeartbeatInterval);

  EXPECT_NE(discovery.HostAddr(), "");
}

//////////////////////////////////////////////////
/// \brief Try to use the discovery features without calling Start().
TEST(DiscoveryTest, WithoutCallingStart)
{
  Discovery<ServicePublisher> discovery(pUuid1, g_ip, g_srvPort);
  ServicePublisher srvPublisher(service, addr1, id1, pUuid1, nUuid1,
    "reqType", "repType", AdvertiseServiceOptions());

  EXPECT_FALSE(discovery.Advertise(srvPublisher));
  EXPECT_FALSE(discovery.Discover(service));
  EXPECT_FALSE(discovery.Unadvertise(service, nUuid1));
}

//////////////////////////////////////////////////
/// \brief Advertise a topic without registering callbacks.
TEST(DiscoveryTest, TestAdvertiseNoResponse)
{
  reset();

  // Create two discovery nodes.
  MsgDiscovery discovery1(pUuid1, g_ip, g_msgPort);
  MsgDiscovery discovery2(pUuid2, g_ip, g_msgPort);

  discovery1.Start();
  discovery2.Start();

  // This should generate discovery traffic but no response on discovery2
  // because there is no callback registered.
  MessagePublisher publisher(g_topic, addr1, ctrl1, pUuid1, nUuid1, "t",
    AdvertiseMessageOptions());
  EXPECT_TRUE(discovery1.Advertise(publisher));

  waitForCallback(MaxIters, Nap, connectionExecuted);

  // Check that the discovery callbacks were not received.
  EXPECT_FALSE(connectionExecuted);
  EXPECT_FALSE(disconnectionExecuted);
}

//////////////////////////////////////////////////
/// \brief Check that the discovery triggers the callbacks after an advertise.
TEST(DiscoveryTest, TestAdvertise)
{
  reset();

  // Create two discovery nodes simulating they are in different processes.
  transport::Discovery<MessagePublisher> discovery1(pUuid1, g_ip, g_msgPort);
  transport::Discovery<MessagePublisher> Discovery2(
    pUuid2, g_ip, g_msgPort, true);

  // Register one callback for receiving notifications.
  Discovery2.ConnectionsCb(onDiscoveryResponse);

  discovery1.Start();
  Discovery2.Start();

  // This should trigger a discovery response on discovery2.
  MessagePublisher publisher(g_topic, addr1, ctrl1, pUuid1, nUuid1, "t",
    AdvertiseMessageOptions());
  EXPECT_TRUE(discovery1.Advertise(publisher));

  waitForCallback(MaxIters, Nap, connectionExecuted);

  EXPECT_TRUE(connectionExecuted);
  EXPECT_FALSE(disconnectionExecuted);

  reset();

  // This should not trigger a discovery response on discovery2. They are in
  // different processes and the scope is set to "Process".
  AdvertiseMessageOptions opts1;
  opts1.SetScope(Scope_t::PROCESS);
  MessagePublisher publisher2("/topic2", addr1, ctrl1, pUuid1, nUuid1,
    "type", opts1);
  EXPECT_TRUE(discovery1.Advertise(publisher2));

  waitForCallback(MaxIters, Nap, connectionExecuted);

  EXPECT_FALSE(connectionExecuted);
  EXPECT_FALSE(disconnectionExecuted);

  reset();

  // This should trigger a discovery response on discovery2.
  AdvertiseMessageOptions opts2;
  opts2.SetScope(Scope_t::HOST);
  MessagePublisher publisher3("/topic3", addr1, ctrl1, pUuid1, nUuid1, "type",
    opts2);
  EXPECT_TRUE(discovery1.Advertise(publisher3));

  waitForCallback(MaxIters, Nap, connectionExecuted);


  EXPECT_TRUE(connectionExecuted);
  EXPECT_FALSE(disconnectionExecuted);
}

//////////////////////////////////////////////////
/// \brief Check that the discovery triggers the callbacks after an advertise.
TEST(DiscoveryTest, TestAdvertiseSameProc)
{
  reset();

  // Create two discovery nodes simulating they are in different processes.
  MsgDiscovery discovery1(pUuid1, g_ip, g_msgPort);
  MsgDiscovery discovery2(pUuid1, g_ip, g_msgPort);

  // Register one callback for receiving notifications.
  discovery2.ConnectionsCb(onDiscoveryResponse);

  discovery1.Start();
  discovery2.Start();

  // This should not trigger a discovery response on discovery2. If the nodes
  // are on the same process, they will not communicate using zeromq.
  MessagePublisher publisher(g_topic, addr1, ctrl1, pUuid1, nUuid1, "t",
    AdvertiseMessageOptions());
  EXPECT_TRUE(discovery1.Advertise(publisher));

  waitForCallback(MaxIters, Nap, connectionExecuted);

  EXPECT_FALSE(connectionExecuted);
  EXPECT_FALSE(disconnectionExecuted);
}

//////////////////////////////////////////////////
/// \brief Check that the discovery triggers the callbacks after a discovery
/// and after register the discovery callback.
TEST(DiscoveryTest, TestDiscover)
{
  reset();

  // Create one discovery node and advertise a topic.
  Discovery<MessagePublisher> discovery1(pUuid1, g_ip, g_msgPort);
  discovery1.Start();

  MessagePublisher publisher(g_topic, addr1, ctrl1, pUuid1, nUuid1, "t",
    AdvertiseMessageOptions());
  EXPECT_TRUE(discovery1.Advertise(publisher));

  // Create a second discovery node that did not see the previous ADV message.
  MsgDiscovery discovery2(pUuid2, g_ip, g_msgPort);
  discovery2.Start();
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // I should not see any discovery updates.
  EXPECT_FALSE(connectionExecuted);
  EXPECT_FALSE(disconnectionExecuted);

  // Register one callback for receiving notifications.
  discovery2.ConnectionsCb(onDiscoveryResponse);

  // Request the discovery of a topic.
  EXPECT_TRUE(discovery2.Discover(g_topic));

  waitForCallback(MaxIters, Nap, connectionExecuted);

  // Check that the discovery response was received.
  EXPECT_TRUE(connectionExecuted);
  EXPECT_FALSE(disconnectionExecuted);

  reset();

  // Request again the discovery of a topic. The callback should be executed
  // from the Discover method this time because the topic information should be
  // known.
  EXPECT_TRUE(discovery2.Discover(g_topic));

  // Check that the discovery response was received.
  EXPECT_TRUE(connectionExecuted);
  EXPECT_FALSE(disconnectionExecuted);
}

//////////////////////////////////////////////////
/// \brief Check that the discovery triggers the disconnection callback after
/// an unadvertise.
TEST(DiscoveryTest, TestUnadvertise)
{
  reset();

  // Create two discovery nodes.
  MsgDiscovery discovery1(pUuid1, g_ip, g_msgPort);
  MsgDiscovery discovery2(pUuid2, g_ip, g_msgPort);

  // Register one callback for receiving disconnect notifications.
  discovery2.DisconnectionsCb(onDisconnection);

  discovery1.Start();
  discovery2.Start();

  // This should not trigger a disconnect response on discovery2.
  MessagePublisher publisher(g_topic, addr1, ctrl1, pUuid1, nUuid1, "t",
    AdvertiseMessageOptions());
  EXPECT_TRUE(discovery1.Advertise(publisher));

  waitForCallback(MaxIters, Nap, disconnectionExecuted);

  // Check that no discovery response was received.
  EXPECT_FALSE(connectionExecuted);
  EXPECT_FALSE(disconnectionExecuted);

  reset();

  // This should trigger a disconnect response on discovery2.
  EXPECT_TRUE(discovery1.Unadvertise(g_topic, nUuid1));

  waitForCallback(MaxIters, Nap, disconnectionExecuted);

  // Check that the discovery response was received.
  EXPECT_FALSE(connectionExecuted);
  EXPECT_TRUE(disconnectionExecuted);

  // Unadvertise a topic not advertised.
  EXPECT_TRUE(discovery1.Unadvertise(g_topic, nUuid1));

  MsgAddresses_M addresses;
  EXPECT_FALSE(discovery2.Publishers(g_topic, addresses));
}

//////////////////////////////////////////////////
/// \brief Check that the discovery triggers the disconnection callback after
/// sending a BYE message (discovery object out of scope).
TEST(DiscoveryTest, TestNodeBye)
{
  reset();

  // Create two discovery nodes.
  std::unique_ptr<MsgDiscovery> discovery1(
    new MsgDiscovery(pUuid1, g_ip, g_msgPort));
  MsgDiscovery discovery2(pUuid2, g_ip, g_msgPort);

  // Register one callback for receiving disconnect notifications.
  discovery2.DisconnectionsCb(onDisconnection);

  discovery1->Start();
  discovery2.Start();

  // This should not trigger a disconnect response on discovery2.
  MessagePublisher publisher(g_topic, addr1, ctrl1, pUuid1, nUuid1, "t",
    AdvertiseMessageOptions());
  EXPECT_TRUE(discovery1->Advertise(publisher));

  waitForCallback(MaxIters, Nap, connectionExecuted);

  // Check that no discovery response was received.
  EXPECT_FALSE(connectionExecuted);
  EXPECT_FALSE(disconnectionExecuted);
  disconnectionExecuted = false;

  // Destroy discovery1. It's destructor should send a BYE message and that
  // should be discovered by discovery2.
  discovery1.reset();

  waitForCallback(MaxIters, Nap, disconnectionExecuted);

  // Check that the discovery response was received.
  EXPECT_FALSE(connectionExecuted);
  EXPECT_TRUE(disconnectionExecuted);
}

//////////////////////////////////////////////////
/// \brief Check that the discovery detects two publishers advertising the same
/// topic name.
TEST(DiscoveryTest, TestTwoPublishersSameTopic)
{
  reset();

  // Create two discovery nodes and advertise the same topic.
  MsgDiscovery discovery1(pUuid1, g_ip, g_msgPort);
  MsgDiscovery discovery2(pUuid2, g_ip, g_msgPort);

  MessagePublisher publisher1(g_topic, addr1, ctrl1, pUuid1, nUuid1, "t",
    AdvertiseMessageOptions());
  MessagePublisher publisher2(g_topic, addr2, ctrl2, pUuid2, nUuid2, "t",
    AdvertiseMessageOptions());

  discovery1.Start();
  discovery2.Start();

  EXPECT_TRUE(discovery1.Advertise(publisher1));
  EXPECT_TRUE(discovery2.Advertise(publisher2));

  // The callbacks should not be triggered but let's wait some time in case
  // something goes wrong.
  std::this_thread::sleep_for(std::chrono::milliseconds(300));

  // I should not see any discovery updates.
  EXPECT_FALSE(connectionExecuted);
  EXPECT_FALSE(disconnectionExecuted);

  // Register one callback for receiving notifications.
  discovery2.ConnectionsCb(onDiscoveryResponseMultiple);

  // Request the discovery of a topic.
  EXPECT_TRUE(discovery2.Discover(g_topic));

  int i = 0;
  while (i < MaxIters && g_counter < 2)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(Nap));
    ++i;
  }

  // Check that the two discovery responses were received.
  EXPECT_TRUE(connectionExecuted);
  EXPECT_FALSE(disconnectionExecuted);
  EXPECT_EQ(g_counter, 2);

  reset();

  // Request again the discovery of a topic. The callback should be executed
  // from the Discover method this time because the topic information should be
  // known.
  EXPECT_TRUE(discovery2.Discover(g_topic));

  // Check that the discovery response was received.
  EXPECT_TRUE(connectionExecuted);
  EXPECT_FALSE(disconnectionExecuted);
  EXPECT_EQ(g_counter, 2);
}

//////////////////////////////////////////////////
/// \brief Check that a discovery service sends messages if there are
/// topics or services advertised in its process.
TEST(DiscoveryTest, TestActivity)
{
  auto proc1Uuid = testing::getRandomNumber();
  auto proc2Uuid = testing::getRandomNumber();
  MessagePublisher publisher(g_topic, addr1, ctrl1, proc1Uuid, nUuid1, "type",
    AdvertiseMessageOptions());
  ServicePublisher srvPublisher(service, addr1, id1, proc2Uuid,
    nUuid2, "reqType", "repType", AdvertiseServiceOptions());
  DiscoveryDerived<MessagePublisher> discovery1(proc1Uuid, g_ip, g_msgPort);

  {
    DiscoveryDerived<MessagePublisher> discovery2(proc2Uuid, g_ip, g_msgPort);

    discovery1.Start();
    discovery2.Start();

    discovery1.Advertise(publisher);

    std::this_thread::sleep_for(std::chrono::milliseconds(
      discovery1.HeartbeatInterval() * 2));

    // We should observe activity from both processes.
    discovery1.TestActivity(proc2Uuid, true);
    discovery2.TestActivity(proc1Uuid, true);

    std::this_thread::sleep_for(std::chrono::milliseconds(
      discovery1.HeartbeatInterval() * 2));
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(
    discovery1.HeartbeatInterval()));

  // We shouldn't observe activity from proc1Uuid2 anymore.
  discovery1.TestActivity(proc2Uuid, false);
}

//////////////////////////////////////////////////
/// \brief Check that a wrong GZ_IP value makes HostAddr() to return 127.0.0.1
TEST(DiscoveryTest, GZ_UTILS_TEST_DISABLED_ON_LINUX(WrongGzIp))
{
  // Save the current value of GZ_IP environment variable.
  std::string gzIp;
  gz::utils::env("GZ_IP", gzIp);

  // Incorrect value for GZ_IP
  ASSERT_TRUE(gz::utils::setenv("GZ_IP", "127.0.0.0"));

  transport::Discovery<MessagePublisher> discovery1(pUuid1, g_ip, g_msgPort);
  EXPECT_EQ(discovery1.HostAddr(), "127.0.0.1");

  // Unset GZ_IP.
  ASSERT_TRUE(gz::utils::unsetenv("GZ_IP"));

  // Restore GZ_IP.
  if (!gzIp.empty())
    gz::utils::setenv("GZ_IP", gzIp);
}
