/*
 * Copyright (C) 2017 Open Source Robotics Foundation
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

#ifndef GZ_TRANSPORT_LOG_MSGITER_HH_
#define GZ_TRANSPORT_LOG_MSGITER_HH_

#include <memory>

#include <gz/transport/config.hh>
#include <gz/transport/log/Export.hh>
#include <gz/transport/log/Message.hh>

namespace gz::transport::log
{
  // Inline bracket to help doxygen filtering.
  inline namespace GZ_TRANSPORT_VERSION_NAMESPACE {
  //
  /// \brief Forward Declarations
  class MsgIterPrivate;
  class Log;

  /// \brief Implements iterator for reading messages
  class GZ_TRANSPORT_LOG_VISIBLE MsgIter
  {
    /// \brief Default constructor
    public: MsgIter();

    /// \brief Move Constructor
    /// \param[in] _orig the instance being copied
    public: MsgIter(MsgIter &&_orig);  // NOLINT

    /// \brief Destructor;
    public: ~MsgIter();

    /// \cond
    /// \brief Copy-assignment operator
    /// \param[in] _orig the instance being copied
    /// \return a reference to this instance
    // public: MsgIter &operator=(const MsgIter &_orig);
    /// \endcond

    /// \brief Prefix increment
    /// \return a reference to this instance
    public: MsgIter &operator++();

    /// \brief Equality operator
    /// \param[in] _other the iterator this is being compared to
    /// \return true if the two iterator point to the same message
    public: bool operator==(const MsgIter &_other) const;

    /// \brief Inequality operator
    /// \param[in] _other the iterator this is being compared to
    /// \return false if the two iterator point to the same message
    public: bool operator!=(const MsgIter &_other) const;

    /// \brief Move assignment operator
    /// \param[in] _other the new iterator replacing the current one
    /// \return The updated MsgIter.
    public: MsgIter& operator=(MsgIter &&_other); // NOLINT

    /// \brief Dereference Operator
    /// \return a reference to the message this is pointing to
    public: const Message &operator*() const;

    /// \brief arrow dereference operator
    /// \return a pointer to the message this is pointing to
    public: const Message *operator->() const;

#ifdef _WIN32
// Disable warning C4251 which is triggered by
// std::*
#pragma warning(push)
#pragma warning(disable: 4251)
#endif
    /// \brief Private Implementation Pointer
    private: std::unique_ptr<MsgIterPrivate> dataPtr;
#ifdef _WIN32
#pragma warning(pop)
#endif

    /// \brief Construct with private implementation
    /// \param[in] _pimpl a private implementation pointer
    /// \internal
    private: MsgIter(
        std::unique_ptr<MsgIterPrivate> &&_pimpl);  // NOLINT(build/c++11)

    /// \brief can use private constructor
    friend class Batch;
  };
  }
}
#endif
