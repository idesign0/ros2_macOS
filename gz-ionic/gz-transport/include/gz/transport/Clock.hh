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
#ifndef GZ_TRANSPORT_CLOCK_HH_
#define GZ_TRANSPORT_CLOCK_HH_

#include <chrono>
#include <memory>
#include <string>

#include <gz/utils/SuppressWarning.hh>

#include "gz/transport/config.hh"
#include "gz/transport/Export.hh"

namespace gz::transport
{
  // Inline bracket to help doxygen filtering.
  inline namespace GZ_TRANSPORT_VERSION_NAMESPACE {
  //
  //////////////////////////////////////////////////
  /// \brief A Clock interface for time tracking
  class GZ_TRANSPORT_VISIBLE Clock
  {
    /// \brief Gets clock time
    /// \return Current clock time, in nanoseconds
    public: virtual std::chrono::nanoseconds Time() const = 0;

    /// \brief Checks whether the clock is ready to be used or not.
    /// \return True if clock is ready to be used, false otherwise
    public: virtual bool IsReady() const = 0;

    /// \brief Virtual destructor
    public: virtual ~Clock() = default;
  };

  ////////////////////////////////////////////////////////////////
  /// \brief A Clock interface implementation that uses
  /// gz::msgs::Clock messages distributed across
  /// the network
  class GZ_TRANSPORT_VISIBLE NetworkClock : public Clock
  {
    /// \brief Network clock time bases
    public: enum class TimeBase : int64_t
    {
      REAL,  ///< Use Clock message `real` time field as time base
      SIM,   ///< Use Clock message `sim` time field as time base
      SYS    ///< Use Clock message `sys` time field as time base
    };

    /// \brief Constructor that sets the initial time range option
    /// \param[in] _topicName Name of the gz::msgs::Clock type
    /// topic to be used
    /// \param[in] _timeBase Time base for this clock, defaults to
    /// simulation time
    public: explicit NetworkClock(const std::string &_topicName,
                                  const TimeBase _timeBase = TimeBase::SIM);

    /// \brief Destructor
    public: ~NetworkClock() override;

    // Documentation inherited
    public: std::chrono::nanoseconds Time() const override;

    /// \brief Sets and distributes the given clock time
    /// \param[in] _time The clock time to be set
    /// \remarks No clock arbitration is performed
    public: void SetTime(const std::chrono::nanoseconds _time);

    // Documentation inherited
    public: bool IsReady() const override;

    /// \internal Implementation of this class
    private: class Implementation;

    /// \internal Pointer to the implementation of this class
    GZ_UTILS_WARN_IGNORE__DLL_INTERFACE_MISSING
    private: std::unique_ptr<Implementation> dataPtr;
    GZ_UTILS_WARN_RESUME__DLL_INTERFACE_MISSING
  };

  //////////////////////////////////////////////////
  /// \brief A Clock implementation that leverages host OS time APIs
  class GZ_TRANSPORT_VISIBLE WallClock : public Clock
  {
    /// \brief Returns system wall clock interface
    /// \return The sole wall clock instance (a singleton)
    public: static WallClock* Instance();

    // Documentation inherited
    public: std::chrono::nanoseconds Time() const override;

    // Documentation inherited
    public: bool IsReady() const override;

    /// \internal Private singleton constructor
    private: WallClock();

    /// \brief Destructor
    private: ~WallClock() override;

    /// \internal Implementation of this class
    private: class Implementation;

    /// \internal Pointer to the implementation of this class
    GZ_UTILS_WARN_IGNORE__DLL_INTERFACE_MISSING
    private: std::unique_ptr<Implementation> dataPtr;
    GZ_UTILS_WARN_RESUME__DLL_INTERFACE_MISSING
  };
  }
}

#endif
