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
#include <gz/msgs/clock.pb.h>
#include <gz/msgs/time.pb.h>

#include <chrono>
#include <string>
#include <thread>

#include "gz/transport/Clock.hh"
#include "gz/transport/Node.hh"
#include "gz/transport/TransportTypes.hh"
#include "gtest/gtest.h"

using namespace gz;

//////////////////////////////////////////////////
/// \brief Check WallClock functionality.
TEST(ClockTest, WallClock)
{
  const transport::WallClock *clock =
      transport::WallClock::Instance();
  ASSERT_TRUE(clock != nullptr);
  EXPECT_TRUE(clock->IsReady());
  const std::chrono::nanoseconds sleepTime(100000000);
  const std::chrono::nanoseconds startTime = clock->Time();
  std::this_thread::sleep_for(sleepTime);
  const std::chrono::nanoseconds endTime = clock->Time();
  // Windows uses system clock for sleep, and transport::WallClock uses
  // steady_clock. This can lead to errors.
#ifdef _WIN32
  const std::chrono::nanoseconds expectedSleepTime(99000000);
#else
  const std::chrono::nanoseconds expectedSleepTime(100000000);
#endif
  EXPECT_GE(endTime - startTime, expectedSleepTime) << "Expected["
    << (endTime-startTime).count() << "] Actual[" << sleepTime.count() << "]\n";
}

//////////////////////////////////////////////////

// Alias for convenience.
using TimeBase = transport::NetworkClock::TimeBase;

/// \brief A test fixture for transport::NetworkClock related tests,
/// parameterized by transport::NetworkClock::TimeBase.
class NetworkClockTest : public ::testing::TestWithParam<TimeBase>
{
  /// \brief Makes an msgs::Clock message out of the
  /// given @p _secs and @p _nsecs.
  /// \param[in] _secs Seconds for the message to be made.
  /// \param[in] _nsecs Nanoseconds for the message to be made.
  /// \return A msgs::Clock message.
  protected: msgs::Clock MakeClockMessage(
      const std::chrono::seconds& _secs,
      const std::chrono::nanoseconds& _nsecs)
  {
    msgs::Clock clockMsg;
    switch (GetParam())
    {
      case TimeBase::SIM:
        clockMsg.mutable_sim()->set_sec(_secs.count());
        clockMsg.mutable_sim()->set_nsec(_nsecs.count());
        clockMsg.mutable_real()->set_sec(_secs.count() * 2);
        clockMsg.mutable_system()->set_sec(_secs.count() * 4);
        break;
      case TimeBase::REAL:
        clockMsg.mutable_sim()->set_sec(_secs.count() / 2);
        clockMsg.mutable_real()->set_sec(_secs.count());
        clockMsg.mutable_real()->set_nsec(_nsecs.count());
        clockMsg.mutable_system()->set_sec(_secs.count() * 2);
        break;
      case TimeBase::SYS:
        clockMsg.mutable_sim()->set_sec(_secs.count() / 4);
        clockMsg.mutable_real()->set_sec(_secs.count() / 2);
        clockMsg.mutable_system()->set_sec(_secs.count());
        clockMsg.mutable_system()->set_nsec(_nsecs.count());
        break;
      default:
        break;
    }
    return clockMsg;
  }
};

//////////////////////////////////////////////////
/// \brief Check NetworkClock functionality.
TEST_P(NetworkClockTest, Functionality)
{
  const std::string clockTopicName{"/clock"};
  transport::NetworkClock clock(clockTopicName, GetParam());
  EXPECT_FALSE(clock.IsReady());
  transport::Node node;
  transport::Node::Publisher clockPub =
      node.Advertise<msgs::Clock>(clockTopicName);
  const std::chrono::milliseconds sleepTime{100};
  clockPub.Publish(msgs::Clock());
  std::this_thread::sleep_for(sleepTime);
  EXPECT_FALSE(clock.IsReady());
  const std::chrono::seconds expectedSecs{54321};
  const std::chrono::nanoseconds expectedNsecs{12345};
  clockPub.Publish(MakeClockMessage(expectedSecs, expectedNsecs));

  // Wait for clock distribution
  std::this_thread::sleep_for(sleepTime);
  EXPECT_TRUE(clock.IsReady());
  EXPECT_EQ(clock.Time(), expectedSecs + expectedNsecs);
  clock.SetTime(expectedSecs + expectedNsecs * 2);

  // Wait for clock distribution
  std::this_thread::sleep_for(sleepTime);
  EXPECT_EQ(clock.Time(), expectedSecs + expectedNsecs * 2);
}

INSTANTIATE_TEST_SUITE_P(TestAllTimeBases, NetworkClockTest,
                        ::testing::Values(TimeBase::SIM,
                                          TimeBase::REAL,
                                          TimeBase::SYS));

/// \brief Check NetworkClock functionality.
TEST(ClockTest, BadNetworkClock)
{
  const std::string badClockTopicName{"//bad-clock"};
  const transport::NetworkClock badTopicClock(badClockTopicName);
  EXPECT_FALSE(badTopicClock.IsReady());
  const transport::NetworkClock::TimeBase badTimebase =
      static_cast<transport::NetworkClock::TimeBase>(-1);
  const std::string clockTopicName{"/clock"};
  transport::NetworkClock badTimebaseClock(clockTopicName, badTimebase);
  EXPECT_FALSE(badTimebaseClock.IsReady());
  transport::Node node;
  transport::Node::Publisher clockPub =
      node.Advertise<msgs::Clock>(clockTopicName);
  const std::chrono::milliseconds sleepTime{100};
  clockPub.Publish(msgs::Clock());
  std::this_thread::sleep_for(sleepTime);
  EXPECT_FALSE(badTimebaseClock.IsReady());
  badTimebaseClock.SetTime(std::chrono::seconds(10));
  std::this_thread::sleep_for(sleepTime);  // Wait for clock distribution
  EXPECT_FALSE(badTimebaseClock.IsReady());
}
