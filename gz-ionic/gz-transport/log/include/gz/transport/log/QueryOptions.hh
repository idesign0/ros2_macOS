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

#ifndef GZ_TRANSPORT_LOG_QUERYOPTIONS_HH_
#define GZ_TRANSPORT_LOG_QUERYOPTIONS_HH_

#include <memory>
#include <regex>
#include <set>
#include <string>
#include <vector>

#include <gz/transport/config.hh>
#include <gz/transport/log/Export.hh>
#include <gz/transport/log/Descriptor.hh>
#include <gz/transport/log/QualifiedTime.hh>
#include <gz/transport/log/SqlStatement.hh>

namespace gz::transport::log
{
  // Inline bracket to help doxygen filtering.
  inline namespace GZ_TRANSPORT_VERSION_NAMESPACE {
  //
  //////////////////////////////////////////////////
  /// \brief The QueryOptions interface is used by Log::QueryMessages() to
  /// determine which messages are retrieved from the log file.
  class GZ_TRANSPORT_LOG_VISIBLE QueryOptions
  {
    /// \brief Generate one or more SQL query statements to be used by the
    /// log file to produce a Batch of messages.
    /// \param[in] _descriptor A Descriptor to help form the SQL statements
    /// \return One or more SQL statements.
    public: virtual std::vector<SqlStatement> GenerateStatements(
      const Descriptor &_descriptor) const = 0;

    /// \brief Get a standard SQL statement preamble, from the SELECT
    /// keyword up to (but not including) the WHERE keyword. This preamble
    /// will make sure that the statement is formatted in a way that MsgIter
    /// will have the information it needs.
    ///
    /// Using this statement without modification will query all messages
    /// in the log, with no specified order (or fail to compile in the SQL
    /// interpreter if you neglect to add a semicolon to the end). Append
    /// the output of StandardMessageQueryClose() to ensure that the query
    /// output will be ordered by the time each message was received, and
    /// that the statement closes with a semicolon.
    ///
    /// \return The initial clause of a standard QueryOptions SQL statement.
    public: static SqlStatement StandardMessageQueryPreamble();

    /// \brief Get a standard ending to a SQL statement that will instruct
    /// the queries to be ordered by the time the messages were received by
    /// the logger. It will also end the statement with a semicolon.
    ///
    /// \return \code{" ORDER BY messages.time_recv;"}\endcode
    public: static SqlStatement StandardMessageQueryClose();

    /// \brief Virtual destructor
    public: virtual ~QueryOptions() = default;
  };

  //////////////////////////////////////////////////
  /// \brief Base class which manages the time range settings for the native
  /// QueryOptions classes.
  class GZ_TRANSPORT_LOG_VISIBLE TimeRangeOption
  {
    /// \brief Constructor that sets the initial time range option.
    /// \param[in] _timeRange The time range.
    public: explicit TimeRangeOption(const QualifiedTimeRange &_timeRange);

    /// \brief Copy constructor
    /// \param[in] _other Another TimeRangeOption
    public: TimeRangeOption(const TimeRangeOption &_other);

    /// \brief Move constructor
    /// \param[in] _other Another TimeRangeOption
    public: TimeRangeOption(TimeRangeOption &&_other);  // NOLINT

    /// \brief Chosen time range
    /// \return A mutable reference to the time range that should be queried
    /// for.
    public: QualifiedTimeRange &TimeRange();

    /// \brief Chosen time range
    /// \return A const reference to the time range that should be queried
    /// for.
    public: const QualifiedTimeRange &TimeRange() const;

    /// \brief Generate a SQL string to represent the time conditions.
    /// This should be appended to a SQL statement after a WHERE keyword.
    /// \return A partial SqlStatement that specifies the time conditions
    /// that this TimeRangeOption has been set with.
    public: SqlStatement GenerateTimeConditions() const;

    /// \brief Destructor
    public: ~TimeRangeOption();

    /// \internal Implementation of this class
    private: class Implementation;

#ifdef _WIN32
// Disable warning C4251 which is triggered by
// std::*
#pragma warning(push)
#pragma warning(disable: 4251)
#endif
    /// \internal Pointer to the implementation of this class
    private: std::unique_ptr<Implementation> dataPtr;
#ifdef _WIN32
#pragma warning(pop)
#endif
  };

  //////////////////////////////////////////////////
  /// \brief Specify a list of topics to query.
  class GZ_TRANSPORT_LOG_VISIBLE TopicList final
      : public virtual QueryOptions,
        public virtual TimeRangeOption
  {
    /// \brief Query for a list of topics over the specified time range (by
    /// default, all time).
    /// \param[in] _topics The topics to include
    /// \param[in] _timeRange The time range to query over
    public: TopicList(
      const std::set<std::string> &_topics = { },
      const QualifiedTimeRange &_timeRange = QualifiedTimeRange::AllTime());

    /// \brief Factory function that accepts any container type that can be
    /// passed through a range-for loop. This will insert the contents of
    /// _topics into a blank TopicList and then return it.
    /// \param[in] _topics The topics to include
    /// \param[in] _timeRange The time range to query over
    public: template <typename Container>
    static TopicList Create(
      const Container &_topics,
      const QualifiedTimeRange &_timeRange = QualifiedTimeRange::AllTime());

    /// \brief Query for a single topic over the specified time range (by
    /// default, all time).
    /// \param[in] _singleTopic The one topic to query for
    /// \param[in] _timeRange The time range to query over
    public: TopicList(
      const std::string &_singleTopic,
      const QualifiedTimeRange &_timeRange = QualifiedTimeRange::AllTime());

    /// \brief Copy constructor
    /// \param[in] _other Another TopicList
    public: TopicList(const TopicList &_other);

    /// \brief Move constructor
    /// \param[in] _other Another TopicList
    public: TopicList(TopicList &&_other);  // NOLINT(whitespace/operators)

    /// \brief Topics of this TopicList
    /// \return A mutable reference to the topics that this TopicList should
    /// query for.
    public: std::set<std::string> &Topics();

    /// \brief Topics of this TopicList
    /// \brief A const reference to the topics that this TopicList should
    /// query for.
    public: const std::set<std::string> &Topics() const;

    // Documentation inherited
    public: std::vector<SqlStatement> GenerateStatements(
      const Descriptor &_descriptor) const override;

    /// \brief Destructor
    public: ~TopicList();

    /// \internal Implementation for this class
    private: class Implementation;

#ifdef _WIN32
// Disable warning C4251 which is triggered by
// std::*
#pragma warning(push)
#pragma warning(disable: 4251)
#endif
    /// \internal Pointer to implementation
    private: std::unique_ptr<Implementation> dataPtr;
#ifdef _WIN32
#pragma warning(pop)
#endif
  };

  //////////////////////////////////////////////////
  /// \brief Specify a pattern of topics to query.
  class GZ_TRANSPORT_LOG_VISIBLE TopicPattern final
      : public virtual QueryOptions,
        public virtual TimeRangeOption
  {
    /// \brief Query for topics that match a pattern, over a specified time
    /// range (by default, all time).
    /// \param[in] _pattern The initial pattern that this option should use
    /// \param[in] _timeRange The initial range of time for this option
    public: TopicPattern(
      const std::regex &_pattern,
      const QualifiedTimeRange &_timeRange = QualifiedTimeRange::AllTime());

    /// \brief Copy constructor
    /// \param[in] _other Another TopicPattern
    public: TopicPattern(const TopicPattern &_other);

    /// \brief Move constructor
    /// \param[in] _other Another TopicPattern
    public: TopicPattern(TopicPattern &&_other);  // NOLINT

    /// \brief Pattern for this option
    /// \return A mutable reference to the regular expression pattern that
    /// this option will query for.
    public: std::regex &Pattern();

    /// \brief Pattern for this option
    /// \return A const reference to the regular expression pattern that
    /// this option will query for.
    public: const std::regex &Pattern() const;

    // Documentation inherited
    public: std::vector<SqlStatement> GenerateStatements(
      const Descriptor &_descriptor) const override;

    /// \brief Destructor
    public: ~TopicPattern();

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

  //////////////////////////////////////////////////
  /// \brief Query for all the topics.
  class GZ_TRANSPORT_LOG_VISIBLE AllTopics final
      : public virtual QueryOptions,
        public virtual TimeRangeOption
  {
    /// \brief Query for all the topics over the specified time range (by
    /// default, all time).
    /// \param[in] _timeRange The initial range of time for this option.
    public: explicit AllTopics(
      const QualifiedTimeRange &_timeRange = QualifiedTimeRange::AllTime());

    /// \brief Copy constructor
    /// \param[in] _other Another AllTopics
    public: AllTopics(const AllTopics &_other);

    /// \brief Move constructor
    /// \param[in] _other Another AllTopics
    public: AllTopics(AllTopics &&_other);  // NOLINT(whitespace/operators)

    // Documentation inherited
    public: std::vector<SqlStatement> GenerateStatements(
      const Descriptor &_descriptor) const override;

    /// \brief Destructor
    public: ~AllTopics();

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

#include <gz/transport/log/detail/QueryOptions.hh>

#endif
