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
#ifndef GZ_TRANSPORT_LOG_RECORDER_HH_
#define GZ_TRANSPORT_LOG_RECORDER_HH_

#include <cstdint>
#include <memory>
#include <regex>
#include <set>
#include <string>

#include <gz/transport/Clock.hh>
#include <gz/transport/config.hh>
#include <gz/transport/log/Export.hh>

namespace gz::transport::log
{
  // Inline bracket to help doxygen filtering.
  inline namespace GZ_TRANSPORT_VERSION_NAMESPACE {
  //
  enum class RecorderError : int64_t
  {
    SUCCESS = 0,
    FAILED_TO_OPEN = -1,
    FAILED_TO_SUBSCRIBE = -2,
    ALREADY_RECORDING = -3,
    INVALID_TOPIC = -4,
    TOPIC_NOT_FOUND = -5,
    ALREADY_SUBSCRIBED_TO_TOPIC = -6,
  };

  /// \brief Records Gazebo Transport topics
  /// This class makes it easy to record topics to a log file.
  /// Responsibilities: topic name matching, time received tracking,
  /// multiple thread safety, subscribing to topics
  class GZ_TRANSPORT_LOG_VISIBLE Recorder
  {
    /// \brief Default constructor
    public: Recorder();

    /// \brief move constructor
    /// \param[in] _old the instance being moved into this one
    public: Recorder(Recorder &&_old);  // NOLINT

    /// \brief destructor
    public: ~Recorder();

    /// \brief Synchronize recording with the given clock.
    /// \param[in] _clockIn clock to synchronize and stamp
    /// incoming messages with
    /// \return SUCCESS if the clock was successfully changed,
    /// ALREADY_RECORDING if a recording is already in progress.
    /// \remarks Clock lifetime must exceed that of this Recorder.
    public: RecorderError Sync(const Clock *_clockIn);

    /// \brief Begin recording topics
    /// \param[in] _file path to log file
    /// \return NO_ERROR if recording was successfully started. If the file
    /// already existed, this will return FAILED_TO_OPEN.
    public: RecorderError Start(const std::string &_file);

    /// \brief Stop recording topics. This function will block if there is
    /// any data in the internal buffer that has not yet been written to
    /// disk.
    public: void Stop();

    /// \brief Add a topic to be recorded (exact match only)
    /// \param[in] _topic The exact topic name
    /// \note This method attempts to subscribe to the topic immediately.
    ///       The subscription will be kept until this is destructed.
    /// \return NO_ERROR if the subscription was created.
    public: RecorderError AddTopic(const std::string &_topic);

    /// \brief Add a topic to be recorded (regex match)
    /// \param[in] _topic Pattern to match against topic names
    /// \note This method attempts to subscribe to topics immediately.
    ///       These subscriptions will be kept until this is destructed.
    ///       New topics that match the pattern will be added as they
    ///       appear, including while recording is active.
    /// \return number of topics subscribed or negative number on error
    public: int64_t AddTopic(const std::regex &_topic);

    /// \brief Get the name of the log file.
    /// \return The name of the log file, or an empty string if Start has
    /// not been successfully called.
    public: std::string Filename() const;

    /// \brief Get the set of topics have have been added.
    /// \return The set of topic names that have been added using the
    /// AddTopic functions.
    public: const std::set<std::string> &Topics() const;

    /// \brief Get the buffer size of the queue that is used to store data
    /// from topic callbacks.
    /// \return Current buffer size in MB.
    public: std::size_t BufferSize() const;

    /// \brief Set the maximum size (in MB) of the buffer that is used to
    /// store data from topic callbacks. When the buffer reaches this size,
    /// the recorder will start dropping older messages to make room for new
    /// ones.
    /// \param[in] _size Buffer size in MB
    public: void SetBufferSize(std::size_t _size);

    /// \internal Implementation of this class
    private: class Implementation;

#ifdef _WIN32
// Disable warning C4251 which is triggered by
// std::*
#pragma warning(push)
#pragma warning(disable: 4251)
#endif
    /// \internal Pointer to the implementation
    private: std::unique_ptr<Implementation> dataPtr;
#ifdef _WIN32
#pragma warning(pop)
#endif
  };
  }
}
#endif
