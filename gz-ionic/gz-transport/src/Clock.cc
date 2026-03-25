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
#include <ctime>
#include <iostream>
#include <mutex>

#include <gz/transport/Clock.hh>
#include <gz/transport/Node.hh>

namespace gz::transport
{
inline namespace GZ_TRANSPORT_VERSION_NAMESPACE {
//////////////////////////////////////////////////
class NetworkClock::Implementation
{
  /// \brief Implementation constructor.
  /// \param[in] _topicName Name of the gz::msgs::Clock type
  /// topic to be used
  /// \param[in] _timeBase Time base for this clock, defaults to
  /// simulation time
  public: Implementation(const std::string &_topicName,
                         const NetworkClock::TimeBase _timeBase);

  /// \brief Gets clock time
  /// \return Current clock time, in nanoseconds
  /// \remarks Reads are synchronized
  public: std::chrono::nanoseconds Time();

  /// \brief Sets and distributes the given clock time
  /// \param[in] _time The clock time to be set
  /// \remarks No clock arbitration is performed
  public: void SetTime(const std::chrono::nanoseconds _time);

  /// \brief Updates current clock time from a message
  /// \param[in] _msg Message to update clock time from
  /// \remarks Writes are synchronized
  public: void UpdateTimeFromMessage(const gz::msgs::Time &_msg);

  /// \brief Clock message subscriber callback.
  /// \param[in] _msg Received clock message
  public: void OnClockMessageReceived(const gz::msgs::Clock &_msg);

  /// \brief Current clock time, in nanoseconds.
  public: std::chrono::nanoseconds clockTimeNS;

  /// \brief Time base to use for the clock.
  public: NetworkClock::TimeBase clockTimeBase;

  /// \brief Lock to synchronize clock accesses.
  public: std::mutex clockMutex;

  /// \brief Node to publish/subscribe clock messages.
  public: Node node;

  /// \brief Publisher to distribute clock messages.
  public: Node::Publisher pub;
};

//////////////////////////////////////////////////
NetworkClock::Implementation::Implementation(const std::string& _topicName,
                                             NetworkClock::TimeBase _timeBase)
    : clockTimeNS(std::chrono::nanoseconds::zero()),
      clockTimeBase(_timeBase)
{
  if (!node.Subscribe(
          _topicName, &Implementation::OnClockMessageReceived, this))
  {
    std::cerr << "Could not subscribe to [" << _topicName << "] topic\n";
  }
  this->pub = node.Advertise<gz::msgs::Clock>(_topicName);
}

//////////////////////////////////////////////////
std::chrono::nanoseconds NetworkClock::Implementation::Time()
{
  std::lock_guard<std::mutex> lock(this->clockMutex);
  return this->clockTimeNS;
}

//////////////////////////////////////////////////
void NetworkClock::Implementation::SetTime(std::chrono::nanoseconds _time)
{
  const std::chrono::seconds timeAsSecs =
      std::chrono::duration_cast<std::chrono::seconds>(_time);
  int secs = timeAsSecs.count();
  int nsecs = (_time -
    std::chrono::duration_cast<std::chrono::nanoseconds>(timeAsSecs)).count();

  gz::msgs::Clock msg;
  switch (this->clockTimeBase)
  {
    case NetworkClock::TimeBase::SIM:
      msg.mutable_sim()->set_sec(secs);
      msg.mutable_sim()->set_nsec(nsecs);
      break;
    case NetworkClock::TimeBase::REAL:
      msg.mutable_real()->set_sec(secs);
      msg.mutable_real()->set_nsec(nsecs);
      break;
    case NetworkClock::TimeBase::SYS:
      msg.mutable_system()->set_sec(secs);
      msg.mutable_system()->set_nsec(nsecs);
      break;
    default:
      std::cerr << "Invalid clock time base\n";
      return;
  }
  // Distributes clock message to every subscriber,
  // including this very same object.
  this->pub.Publish(msg);
}

//////////////////////////////////////////////////
void NetworkClock::Implementation::UpdateTimeFromMessage(
    const gz::msgs::Time& msg)
{
  std::lock_guard<std::mutex> lock(this->clockMutex);
  this->clockTimeNS = std::chrono::seconds(msg.sec()) +
                      std::chrono::nanoseconds(msg.nsec());
}

//////////////////////////////////////////////////
void NetworkClock::Implementation::OnClockMessageReceived(
    const gz::msgs::Clock& msg)
{
  switch (this->clockTimeBase)
  {
    case NetworkClock::TimeBase::REAL:
      if (msg.has_real())
      {
        this->UpdateTimeFromMessage(msg.real());
      }
      else
      {
        std::cerr << "Real time not present in clock message\n";
      }
      break;
    case NetworkClock::TimeBase::SIM:
      if (msg.has_sim())
      {
        this->UpdateTimeFromMessage(msg.sim());
      }
      else
      {
        std::cerr << "Sim time not present in clock message\n";
      }
      break;
    case NetworkClock::TimeBase::SYS:
      if (msg.has_system())
      {
        this->UpdateTimeFromMessage(msg.system());
      }
      else
      {
        std::cerr << "System time not present in clock message\n";
      }
      break;
    default:
      std::cerr << "Invalid clock time base\n";
      break;
  }
}

//////////////////////////////////////////////////
NetworkClock::NetworkClock(const std::string &_topicName, TimeBase _timeBase)
    : dataPtr(new NetworkClock::Implementation(_topicName, _timeBase))
{
}

//////////////////////////////////////////////////
NetworkClock::~NetworkClock()
{
  // Destroys the pimpl
}

//////////////////////////////////////////////////
std::chrono::nanoseconds NetworkClock::Time() const {
  return this->dataPtr->Time();
}

//////////////////////////////////////////////////
void NetworkClock::SetTime(std::chrono::nanoseconds _time) {
  return this->dataPtr->SetTime(_time);
}

//////////////////////////////////////////////////
bool NetworkClock::IsReady() const {
  // At least one non-zero clock message has arrived.
  return (this->dataPtr->Time().count() != 0);
}

//////////////////////////////////////////////////
class WallClock::Implementation
{
  /// \brief Default constructor
  public: Implementation();

  /// \brief Gets clock time
  /// \return Current clock time, in nanoseconds
  public: std::chrono::nanoseconds Time() const;

  /// \brief Offset duration for a monotonic clock to
  /// become an UTC one, in nanoseconds
  public: std::chrono::nanoseconds wallMinusMono;
};

//////////////////////////////////////////////////
WallClock::Implementation::Implementation()
{
  // Set the offset used to get UTC from steady clock
  const std::chrono::nanoseconds wallStartNS(
      std::chrono::seconds(std::time(NULL)));
  const std::chrono::nanoseconds monoStartNS(
      std::chrono::steady_clock::now().time_since_epoch());
  this->wallMinusMono = wallStartNS - monoStartNS;
}

//////////////////////////////////////////////////
std::chrono::nanoseconds WallClock::Implementation::Time() const
{
  // Get time RX using monotonic
  const std::chrono::nanoseconds nowNS(
      std::chrono::steady_clock::now().time_since_epoch());
  // monotonic -> utc in nanoseconds
  return this->wallMinusMono + nowNS;
}

//////////////////////////////////////////////////
WallClock* WallClock::Instance()
{
  static WallClock clock;
  return &clock;
}

//////////////////////////////////////////////////
WallClock::WallClock()
    : dataPtr(new WallClock::Implementation)
{
}

//////////////////////////////////////////////////
WallClock::~WallClock()
{
  // Destroys the pimpl
}

//////////////////////////////////////////////////
std::chrono::nanoseconds WallClock::Time() const
{
  return this->dataPtr->Time();
}

//////////////////////////////////////////////////
bool WallClock::IsReady() const
{
  return true;  // Always ready.
}
}  // GZ_TRANSPORT_VERSION_NAMESPACE
}  // namespace gz::transport
