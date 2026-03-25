/*
 * Copyright (C) 2016 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <gz/common/Timer.hh>

using namespace gz;
using namespace common;

/// \brief Private data for the Timer class
class gz::common::Timer::Implementation
{
  /// \brief The time of the last call to Start
  public: std::chrono::steady_clock::time_point start;

  /// \brief The time when Stop was called.
  public: std::chrono::steady_clock::time_point stop;

  /// \brief True if the timer is running.
  public: bool running {false};
};

//////////////////////////////////////////////////
Timer::Timer()
  : dataPtr(gz::utils::MakeImpl<Implementation>())
{
}

//////////////////////////////////////////////////
void Timer::Start()
{
  this->dataPtr->start = std::chrono::steady_clock::now();
  this->dataPtr->running = true;
}

//////////////////////////////////////////////////
void Timer::Stop()
{
  this->dataPtr->stop = std::chrono::steady_clock::now();
  this->dataPtr->running = false;
}

//////////////////////////////////////////////////
bool Timer::Running() const
{
  return this->dataPtr->running;
}

//////////////////////////////////////////////////
std::chrono::duration<double> Timer::ElapsedTime() const
{
  if (this->dataPtr->running)
  {
    std::chrono::steady_clock::time_point currentTime;
    currentTime = std::chrono::steady_clock::now();
    std::chrono::duration<double> diff = currentTime - this->dataPtr->start;
    return diff;
  }
  else
  {
    std::chrono::duration<double> diff =
      this->dataPtr->stop - this->dataPtr->start;
    return diff;
  }
}
