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

#ifndef GZ_TRANSPORT_LOG_BATCH_HH_
#define GZ_TRANSPORT_LOG_BATCH_HH_

#include <memory>

#include <gz/transport/config.hh>
#include <gz/transport/log/Export.hh>
#include <gz/transport/log/MsgIter.hh>

namespace gz::transport::log
{
  // Inline bracket to help doxygen filtering.
  inline namespace GZ_TRANSPORT_VERSION_NAMESPACE {
  //
  /// \brief Forward declaration
  class BatchPrivate;
  class Log;

  /// \brief Holds the result of a query for messages
  class GZ_TRANSPORT_LOG_VISIBLE Batch
  {
    /// \brief Default constructor
    public: Batch();

    /// \brief move constructor
    /// \param[in] _old the instance being moved into this one
    public: Batch(Batch &&_old);  // NOLINT

    /// \brief Move assignment operator
    /// \param[in] _other the new Batch replacing the current one
    /// \return The updated Batch instance.
    public: Batch& operator=(Batch &&_other); // NOLINT

    /// \brief No copy constructor.
    public: Batch(const Batch& other) = delete;

    /// \brief No copy-assignment operator.
    public: Batch& operator=(const Batch& other) = delete;

    /// \brief destructor
    public: ~Batch();

    /// \brief typedef for prettiness
    public: using iterator = MsgIter;

    /// \brief Iterator to first message in batch
    /// \remarks the lowercase function name is required to support
    ///   range-based for loops
    /// \return an iterator to the start of the messages
    public: iterator begin();

    /// \brief Iterator to one past the last message in a batch
    /// \remarks the lowercase function name is required to support
    ///   range-based for loops
    /// \return an iterator that is not equal to any iterator that points
    ///   to a valid message
    public: iterator end();

#ifdef _WIN32
// Disable warning C4251 which is triggered by
// std::*
#pragma warning(push)
#pragma warning(disable: 4251)
#endif
    /// \brief Private implementation
    private: std::unique_ptr<BatchPrivate> dataPtr;
#ifdef _WIN32
#pragma warning(pop)
#endif

    /// \brief Construct with private implementation
    /// \param[in] _pimpl a private implementation pointer
    /// \internal
    private: Batch(
        std::unique_ptr<BatchPrivate> &&_pimpl);  // NOLINT(build/c++11)

    /// \brief Log can use private constructor
    friend class Log;
  };
  }
}
#endif
