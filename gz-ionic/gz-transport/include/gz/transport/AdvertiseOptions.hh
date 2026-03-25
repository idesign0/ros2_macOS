/*
 * Copyright (C) 2015 Open Source Robotics Foundation
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

#ifndef GZ_TRANSPORT_ADVERTISEOPTIONS_HH_
#define GZ_TRANSPORT_ADVERTISEOPTIONS_HH_

#include <cstdint>
#include <iostream>
#include <memory>

#include "gz/transport/config.hh"
#include "gz/transport/Export.hh"

namespace gz::transport
{
  // Inline bracket to help doxygen filtering.
  inline namespace GZ_TRANSPORT_VERSION_NAMESPACE {
  //
  // Forward declarations.
  class AdvertiseOptionsPrivate;
  class AdvertiseMessageOptionsPrivate;
  class AdvertiseServiceOptionsPrivate;

  /// \brief This strongly typed enum defines the different options for
  /// the scope of a topic/service.
  enum class Scope_t
  {
    /// \brief Topic/service only available to subscribers in the same
    /// process as the publisher.
    PROCESS,
    /// \brief Topic/service only available to subscribers in the same
    /// machine as the publisher.
    HOST,
    /// \brief Topic/service available to any subscriber (default scope).
    ALL
  };

  /// \class AdvertiseOptions AdvertiseOptions.hh
  /// gz/transport/AdvertiseOptions.hh
  /// \brief A class for customizing the publication options for a topic or
  /// service advertised.
  /// E.g.: Set the scope of a topic/service.
  class GZ_TRANSPORT_VISIBLE AdvertiseOptions
  {
    /// \brief Constructor.
    public: AdvertiseOptions();

    /// \brief Copy constructor.
    /// \param[in] _other AdvertiseOptions to copy.
    public: AdvertiseOptions(const AdvertiseOptions &_other);

    /// \brief Destructor.
    public: virtual ~AdvertiseOptions();

    /// \brief Assignment operator.
    /// \param[in] _other The new AdvertiseOptions.
    /// \return A reference to this instance.
    public: AdvertiseOptions &operator=(const AdvertiseOptions &_other);

    /// \brief Equality operator. This function checks if the given
    /// AdvertiseOptions has identical content to this object.
    /// \param[in] _other The options to compare against.
    /// \return True if this object matches the provided object.
    public: bool operator==(const AdvertiseOptions &_other) const;

    /// \brief Inequality operator. This function checks if the given
    /// options do not have identical values to this object.
    /// \param[in] _other The options to compare against.
    /// \return True if this object does not match the provided object.
    public: bool operator!=(const AdvertiseOptions &_other) const;

    /// \brief Stream insertion operator.
    /// \param[out] _out The output stream.
    /// \param[in] _other AdvertiseOptions to write to the stream.
    public: friend std::ostream &operator<<(std::ostream &_out,
                                            const AdvertiseOptions &_other)
    {
      _out << "Advertise options:\n"
           << "\tScope: ";
      if (_other.Scope() == Scope_t::PROCESS)
        _out << "Process" << std::endl;
      else if (_other.Scope() == Scope_t::HOST)
        _out << "Host" << std::endl;
      else
        _out << "All" << std::endl;
      return _out;
    }

    /// \brief Get the scope used in this topic/service.
    /// \return The scope.
    /// \sa SetScope.
    /// \sa Scope_t.
    public: const Scope_t &Scope() const;

    /// \brief Set the scope of the topic or service.
    /// \param[in] _scope The new scope.
    /// \sa Scope.
    /// \sa Scope_t.
    public: void SetScope(const Scope_t &_scope);

#ifdef _WIN32
// Disable warning C4251 which is triggered by
// std::unique_ptr
#pragma warning(push)
#pragma warning(disable: 4251)
#endif
    /// \internal
    /// \brief Smart pointer to private data.
    private: std::unique_ptr<AdvertiseOptionsPrivate> dataPtr;
#ifdef _WIN32
#pragma warning(pop)
#endif
  };

  /// \brief A class for customizing the publication options for a topic
  /// advertised.
  /// E.g.: Set the rate of messages per second published.
  class GZ_TRANSPORT_VISIBLE AdvertiseMessageOptions
    : public AdvertiseOptions
  {
    /// \brief Constructor.
    public: AdvertiseMessageOptions();

    /// \brief Copy constructor.
    /// \param[in] _other AdvertiseMessageOptions to copy.
    public: AdvertiseMessageOptions(const AdvertiseMessageOptions &_other);

    /// \brief Destructor.
    public: virtual ~AdvertiseMessageOptions();

    /// \brief Assignment operator.
    /// \param[in] _other The other AdvertiseMessageOptions.
    /// \return A reference to this instance.
    public: AdvertiseMessageOptions &operator=(
      const AdvertiseMessageOptions &_other);

    /// \brief Equality operator. This function checks if the given
    /// AdvertiseMessageOptions has identical content to this object.
    /// \param[in] _other The options to compare against.
    /// \return True if this object matches the provided object.
    public: bool operator==(const AdvertiseMessageOptions &_other) const;

    /// \brief Inequality operator. This function checks if the given
    /// options do not have identical values to this object.
    /// \param[in] _other The options to compare against.
    /// \return True if this object does not match the provided object.
    public: bool operator!=(const AdvertiseMessageOptions &_other) const;

    /// \brief Stream insertion operator.
    /// \param[out] _out The output stream.
    /// \param[in] _other AdvertiseMessageOptions to write to the stream.
    public: friend std::ostream &operator<<(std::ostream &_out,
                                        const AdvertiseMessageOptions &_other)
    {
      _out << static_cast<AdvertiseOptions>(_other);
      if (_other.Throttled())
      {
        _out << "\tThrottled? Yes" << std::endl;
        _out << "\tRate: " << _other.MsgsPerSec() << " msgs/sec" << std::endl;
      }
      else
        _out << "\tThrottled? No" << std::endl;

      return _out;
    }

    /// \brief Whether the publication has been throttled.
    /// \return true when the publication is throttled or false otherwise.
    /// \sa SetMsgsPerSec
    /// \sa MsgsPerSec
    public: bool Throttled() const;

    /// \brief Get the maximum number of messages per second to be published.
    /// \return The maximum number of messages per second.
    public: uint64_t MsgsPerSec() const;

    /// \brief Set the maximum number of messages per second to be published.
    /// Note that we calculate the minimum period of a message based
    /// on the msgs/sec rate. Any message sent since the last Publish()
    /// and the duration of the period will be discarded.
    /// \param[in] _newMsgsPerSec Maximum number of messages per second.
    public: void SetMsgsPerSec(const uint64_t _newMsgsPerSec);

#ifdef _WIN32
// Disable warning C4251 which is triggered by
// std::unique_ptr
#pragma warning(push)
#pragma warning(disable: 4251)
#endif
    /// \internal
    /// \brief Smart pointer to private data.
    private: std::unique_ptr<AdvertiseMessageOptionsPrivate> dataPtr;
#ifdef _WIN32
#pragma warning(pop)
#endif
  };

  /// \brief A class for customizing the publication options for a service
  /// advertised.
  class GZ_TRANSPORT_VISIBLE AdvertiseServiceOptions
    : public AdvertiseOptions
  {
    /// \brief Constructor.
    public: AdvertiseServiceOptions();

    /// \brief Copy constructor.
    /// \param[in] _other AdvertiseServiceOptions to copy.
    public: AdvertiseServiceOptions(const AdvertiseServiceOptions &_other);

    /// \brief Destructor.
    public: virtual ~AdvertiseServiceOptions();

    /// \brief Assignment operator.
    /// \param[in] _other The other AdvertiseServiceOptions.
    /// \return A reference to this instance.
    public: AdvertiseServiceOptions &operator=(
      const AdvertiseServiceOptions &_other);

    /// \brief Equality operator. This function checks if the given
    /// AdvertiseMessageOptions has identical content to this object.
    /// \param[in] _other The options to compare against.
    /// \return True if this object matches the provided object.
    public: bool operator==(const AdvertiseServiceOptions &_other) const;

    /// \brief Inequality operator. This function checks if the given
    /// options do not have identical values to this object.
    /// \param[in] _other The options to compare against.
    /// \return True if this object does not match the provided object.
    public: bool operator!=(const AdvertiseServiceOptions &_other) const;

    /// \brief Stream insertion operator.
    /// \param[out] _out The output stream.
    /// \param[in] _other AdvertiseServiceOptions to write to the stream.
    public: friend std::ostream &operator<<(std::ostream &_out,
                                        const AdvertiseServiceOptions &_other)
    {
      _out << static_cast<AdvertiseOptions>(_other);
      return _out;
    }

#ifdef _WIN32
// Disable warning C4251 which is triggered by
// std::unique_ptr
#pragma warning(push)
#pragma warning(disable: 4251)
#endif
    /// \internal
    /// \brief Smart pointer to private data.
    private: std::unique_ptr<AdvertiseServiceOptionsPrivate> dataPtr;
#ifdef _WIN32
#pragma warning(pop)
#endif
  };
  }
}
#endif
