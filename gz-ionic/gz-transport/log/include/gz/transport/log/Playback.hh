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
#ifndef GZ_TRANSPORT_LOG_PLAYBACK_HH_
#define GZ_TRANSPORT_LOG_PLAYBACK_HH_

#include <chrono>
#include <memory>
#include <regex>
#include <string>

#include <gz/transport/config.hh>
#include <gz/transport/log/Export.hh>
#include <gz/transport/NodeOptions.hh>

namespace gz::transport::log
{
  // Inline bracket to help doxygen filtering.
  inline namespace GZ_TRANSPORT_VERSION_NAMESPACE {
  //
  // Forward declarations
  class PlaybackHandle;
  using PlaybackHandlePtr = std::shared_ptr<PlaybackHandle>;

  //////////////////////////////////////////////////
  /// \brief Initiates playback of Gazebo Transport topics
  /// This class makes it easy to play topics from a log file
  ///
  /// Responsibilities: topic name matching and initiating the playback
  class GZ_TRANSPORT_LOG_VISIBLE Playback
  {
    /// \brief Constructor
    /// \param[in] _file path to log file
    /// \param[in] _nodeOptions Options for creating a node.
    public: explicit Playback(const std::string &_file,
                              const NodeOptions &_nodeOptions = NodeOptions());

    /// \brief move constructor
    /// \param[in] _old the instance being moved into this one
    public: Playback(Playback &&_old);  // NOLINT

    /// \brief destructor
    public: ~Playback();

    /// \brief Begin playing messages
    /// \param[in] _waitAfterAdvertising How long to wait before the
    /// publications begin after advertising the topics that will be played
    /// back.
    /// \param[in] _msgWaiting True to wait between publication of
    /// messages based on the message timestamps. False to playback
    /// messages as fast as possible. Default value is true.
    ///
    /// \note The topic discovery process will need some time before
    /// publishing begins, or else subscribers in other processes will miss
    /// the outgoing messages. The default value is recommended unless you
    /// are confident in the timing of your system.
    ///
    /// \remark If your application uses another library that uses sqlite3,
    /// it may be unsafe to start multiple simultaneous PlaybackHandles from
    /// the same Playback instance, because they will interact with the same
    /// sqlite3 database in multiple threads (see https://www.sqlite.org/threadsafe.html).
    /// In most cases there should be no issue, but if you or a library you
    /// are using calls sqlite3_config(~) to change the threading mode to
    /// Single-thread or Multi-thread (instead of the default setting of
    /// Serialized), then starting multiple simultaneous playbacks from the
    /// same log file could be dangerous.
    ///
    /// \return A handle for managing the playback of the log. You must hold
    /// onto this object in order for the playback to continue, or else it
    /// will be cut short.
    ///
    /// If an error prevents the playback from starting, this will return a
    /// nullptr.
    public: [[nodiscard]] PlaybackHandlePtr Start(
      const std::chrono::nanoseconds &_waitAfterAdvertising =
        std::chrono::seconds(1),
        bool _msgWaiting = true) const;

    /// \brief Check if this Playback object has a valid log to play back
    /// \return true if this has a valid log to play back, otherwise false.
    public: bool Valid() const;

    /// \brief Add a topic to be played back (exact match only)
    /// \param[in] _topic The exact topic name
    /// \note This method attempts to advertise the topic immediately.
    ///       The publisher will be kept until this is destructed.
    /// \return True if the topic exists in the log, false if it does not
    /// exist or if the log is not valid.
    public: bool AddTopic(const std::string &_topic);

    /// \brief Add a topic to be played back (regex match)
    /// \param[in] _topic Pattern to match against topic names
    /// \note This method attempts to advertise topics immediately.
    ///       These publishers will be kept until this is destructed.
    /// \return number of topics that will be published or -1 if this
    /// Playback object is not valid.
    public: int64_t AddTopic(const std::regex &_topic);

    /// \brief Remove a topic from being played back.
    ///
    /// \note If neither AddTopic() nor RemoveTopic() were called prior to
    /// this function, we will assume you want all of the topics and then
    /// remove the one passed in here.
    ///
    /// \param[in] _topic The exact topic name to remove
    /// \return True if the topic was in the list and has been removed,
    /// false otherwise.
    public: bool RemoveTopic(const std::string &_topic);

    /// \brief Remove all topics matching the specified pattern from being
    /// played back.
    ///
    /// \note If neither AddTopic() nor RemoveTopic() were called prior to
    /// this function, we will assume you want all of the topics and then
    /// remove the one passed in here.
    ///
    /// \param[in] _topic The pattern of topics to be removed
    /// \return The number of topics that have been removed from playback
    /// due to this function call.
    public: int64_t RemoveTopic(const std::regex &_topic);

    /// \internal Implementation of this class
    private: class Implementation;

#ifdef _WIN32
// Disable warning C4251 which is triggered by
// std::*
#pragma warning(push)
#pragma warning(disable: 4251)
#endif
    /// \brief Private implementation
    private: std::unique_ptr<Implementation> dataPtr;
#ifdef _WIN32
#pragma warning(pop)
#endif
  };

  //////////////////////////////////////////////////
  /// \brief Handles the playback of Gazebo Transport topics.
  /// This class allows you to manage a log playback once it has started.
  /// You must hang onto the PlaybackHandle or else the playback will end
  /// early.
  ///
  /// Responsibilities: time keeping, multiple thread safety, publishing
  /// data to topics, and stopping playback.
  class GZ_TRANSPORT_LOG_VISIBLE PlaybackHandle
  {
    /// \brief Stop playing messages
    public: void Stop();

    /// \brief Jump current playback time to a specific elapsed time
    /// \param[in] _newElapsedTime Elapsed time at which playback will jump
    public: void Seek(const std::chrono::nanoseconds &_newElapsedTime);

    /// \brief Step the playback by a given amount of nanoseconds
    /// \pre Playback must be previously paused
    /// \param[in] _stepDuration Length of the step in nanoseconds
    public: void Step(const std::chrono::nanoseconds &_stepDuration);

    /// \brief Pauses the playback
    public: void Pause();

    /// \brief Unpauses the playback
    public: void Resume();

    /// \brief Check pause status
    public: bool IsPaused() const;

    /// \brief Block until playback runs out of messages to publish
    public: void WaitUntilFinished();

    /// \brief Check if this playback is finished
    /// \return true if all messages have finished playing; false otherwise.
    public: bool Finished() const;

    /// \brief Gets start time of the log being played
    /// \return start time of the log, in nanoseconds
    public: std::chrono::nanoseconds StartTime() const;

    /// \brief Gets current time of the log being played
    /// \return current time of the log playback, in nanoseconds
    public: std::chrono::nanoseconds CurrentTime() const;

    /// \brief Gets end time of the log being played
    /// \return end time of the log, in nanoseconds
    public: std::chrono::nanoseconds EndTime() const;

    /// \brief Destructor
    public: ~PlaybackHandle();

    // Friendship
    friend class Playback;

    // Forward declaration of implementation class
    private: class Implementation;

    /// \brief Private constructor. This can only be called by Playback.
    private: PlaybackHandle(
      std::unique_ptr<Implementation> &&_internal); // NOLINT

#ifdef _WIN32
// Disable warning C4251 which is triggered by
// std::*
#pragma warning(push)
#pragma warning(disable: 4251)
#endif
    /// \internal Pointer to implementation class
    private: std::unique_ptr<Implementation> dataPtr;
#ifdef _WIN32
#pragma warning(pop)
#endif
  };
  }
}
#endif
