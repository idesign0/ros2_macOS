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

#ifndef GZ_TRANSPORT_LOG_LOG_HH_
#define GZ_TRANSPORT_LOG_LOG_HH_

#include <chrono>
#include <ios>
#include <memory>
#include <string>

#include <gz/transport/config.hh>
#include <gz/transport/log/Batch.hh>
#include <gz/transport/log/QueryOptions.hh>
#include <gz/transport/log/Descriptor.hh>
#include <gz/transport/log/Export.hh>

namespace gz::transport::log
{
  // Inline bracket to help doxygen filtering.
  inline namespace GZ_TRANSPORT_VERSION_NAMESPACE {
  //
  /// \brief Name of Environment variable containing path to schema
  const std::string SchemaLocationEnvVar = "GZ_TRANSPORT_LOG_SQL_PATH";

  /// \brief Interface to a log file
  class GZ_TRANSPORT_LOG_VISIBLE Log
  {
    /// \brief constructor
    public: Log();

    /// \brief move constructor
    /// \param[in] _old the instance being moved into this one
    public: Log(Log &&_old);  // NOLINT

    /// \brief destructor
    public: ~Log();

    /// \brief Indicate if a log has been successfully opened
    /// \return true if a log is open
    public: bool Valid() const;

    /// \brief Get the schema version of the opened log
    /// \return the current version of the schema in the log file
    /// \return empty string if the log has not been opened
    public: std::string Version() const;

    /// \brief Open a log file
    /// \param[in] _file path to log file
    /// \param[in] _mode flag indicating read only or read/write
    ///   Can use (in or out)
    /// \return True if the log file was successfully opened, false otherwise.
    public: bool Open(const std::string &_file,
        std::ios_base::openmode _mode = std::ios_base::in);

    /// \brief Get the name of the log file.
    /// \return The name of the log file, or an empty string if Open has
    /// not been successfully called.
    public: std::string Filename() const;

    /// \brief Get a Descriptor for this log. The Descriptor will be
    /// generated the first time that this function is called after a new
    /// file has been opened.
    /// \return A Descriptor for this log, if a log file is currently
    /// loaded. If no log file is loaded, this returns a nullptr.
    public: const log::Descriptor *Descriptor() const;

    /// \brief Insert a message into the log file
    /// \param[in] _time Time the message was received (ns since Unix epoch)
    /// \param[in] _topic Name of the topic the message was on
    /// \param[in] _type Name of the message type
    /// \param[in] _data pointer to a buffer containing the message data
    /// \param[in] _len number of bytes of data
    /// \return true if the message was successfully inserted
    public: bool InsertMessage(
        const std::chrono::nanoseconds &_time,
        const std::string &_topic, const std::string &_type,
        const void *_data, std::size_t _len);

    /// \brief Get messages according to the specified options. By default,
    /// it will query all messages over the entire time range of the log.
    /// \param[in] _options A QueryOptions type to indicate what kind of
    /// messages you would like to query.
    /// \return A Batch which matches the requested QueryOptions.
    public: Batch QueryMessages(const QueryOptions &_options = AllTopics());

    /// \brief Get start time of the log, or in other words the
    /// time of the first message found in the log
    /// \return start time of the log, or zero if the log is not
    /// valid or if data retrieval failed.
    public: std::chrono::nanoseconds StartTime() const;

    /// \brief Get end time of the log, or in other words the
    /// time of the last message found in the log
    /// \return end time of the log, or zero if the log is not
    /// valid or if data retrieval failed.
    public: std::chrono::nanoseconds EndTime() const;

    /// \internal Implementation for this class
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
  }
}
#endif
