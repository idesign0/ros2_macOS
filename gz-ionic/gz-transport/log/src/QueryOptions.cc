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

#include <cstdint>
#include <regex>
#include <set>
#include <string>
#include <vector>

#include <gz/transport/log/QueryOptions.hh>

using namespace gz::transport;
using namespace gz::transport::log;

//////////////////////////////////////////////////
/// \brief Append a topic ID condition clause that specifies a list of Topic IDs
/// \param[in,out] _sql The SqlStatement to append the clause to
/// \param[in] _ids The vector of Topic IDs to include in the list
static void AppendTopicListClause(
    SqlStatement &_sql, const std::vector<int64_t> &_ids)
{
  _sql.statement += "topic_id in (";
  bool first = true;
  for (const int64_t id : _ids)
  {
    if (first)
    {
      _sql.statement += "?";
      first = false;
    }
    else
    {
      _sql.statement += ", ?";
    }

    _sql.parameters.emplace_back(id);
  }

  _sql.statement += ")";
}

//////////////////////////////////////////////////
SqlStatement QueryOptions::StandardMessageQueryPreamble()
{
  SqlStatement sql;
  sql.statement =
      "SELECT messages.id, messages.time_recv, topics.name,"
      " message_types.name, messages.message FROM messages JOIN topics ON"
      " topics.id = messages.topic_id JOIN message_types ON"
      " message_types.id = topics.message_type_id ";

  return sql;
}

//////////////////////////////////////////////////
SqlStatement QueryOptions::StandardMessageQueryClose()
{
  return SqlStatement{ " ORDER BY messages.time_recv;", {} };
}

//////////////////////////////////////////////////
class TimeRangeOption::Implementation
{
  /// \brief Convert the QualifiedTimeRange into a SqlStatement clause that
  /// can be appended to the complete clause.
  /// \return SqlStatement time range clause
  public: SqlStatement GenerateTimeConditions() const
  {
    SqlStatement sql;

    const QualifiedTime &start = this->range.Beginning();
    const QualifiedTime &finish = this->range.Ending();

    if (start.IsIndeterminate() && finish.IsIndeterminate())
    {
      // We return a blank SQL statement, because we have no time constraints.
      return SqlStatement();
    }

    std::string startCompare;
    std::string finishCompare;

    if (!start.IsIndeterminate())
    {
      if (*start.GetQualifier() == QualifiedTime::Qualifier::INCLUSIVE)
        startCompare = ">=";
      else if (*start.GetQualifier() == QualifiedTime::Qualifier::EXCLUSIVE)
        startCompare = ">";
    }

    if (!finish.IsIndeterminate())
    {
      if (*finish.GetQualifier() == QualifiedTime::Qualifier::INCLUSIVE)
        finishCompare = "<=";
      else if (*finish.GetQualifier() == QualifiedTime::Qualifier::EXCLUSIVE)
        finishCompare = "<";
    }

    if (!startCompare.empty())
    {
      sql.statement += "time_recv " + startCompare + " ?";
      sql.parameters.emplace_back(static_cast<int64_t>(
          start.GetTime()->count()));

      if (!finishCompare.empty())
        sql.statement += " AND ";
    }

    if (!finishCompare.empty())
    {
      sql.statement += "time_recv " + finishCompare + " ?";
      sql.parameters.emplace_back(static_cast<int64_t>(
          finish.GetTime()->count()));
    }

    return sql;
  }

  /// \brief Range for this option
  public: QualifiedTimeRange range;
};

//////////////////////////////////////////////////
TimeRangeOption::TimeRangeOption(const QualifiedTimeRange &_timeRange)
  : dataPtr(new Implementation{_timeRange})
{
  // Do nothing
}

//////////////////////////////////////////////////
TimeRangeOption::TimeRangeOption(const TimeRangeOption &_other)
  : dataPtr(new Implementation{*_other.dataPtr})
{
  // Do nothing
}

//////////////////////////////////////////////////
TimeRangeOption::TimeRangeOption(
    TimeRangeOption &&_other)  // NOLINT(build/c++11)
  : dataPtr(std::move(_other.dataPtr))
{
  // Do nothing
}

//////////////////////////////////////////////////
QualifiedTimeRange &TimeRangeOption::TimeRange()
{
  return this->dataPtr->range;
}

//////////////////////////////////////////////////
const QualifiedTimeRange &TimeRangeOption::TimeRange() const
{
  return this->dataPtr->range;
}

//////////////////////////////////////////////////
SqlStatement TimeRangeOption::GenerateTimeConditions() const
{
  return this->dataPtr->GenerateTimeConditions();
}

//////////////////////////////////////////////////
TimeRangeOption::~TimeRangeOption()
{
  // Destroy the pimpl
}

//////////////////////////////////////////////////
class TopicList::Implementation
{
  /// \brief Generate a WHERE clause for topics that exist in the requested list
  /// \param[in] _descriptor The descriptor forwarded by the interface class
  /// \return The desired WHERE clause
  public: SqlStatement GenerateStatement(
    const Descriptor &_descriptor)
  {
    const Descriptor::NameToMap &map = _descriptor.TopicsToMsgTypesToId();
    std::vector<int64_t> rowIDs;
    rowIDs.reserve(map.size());

    for (const auto &topic : topics)
    {
      Descriptor::NameToMap::const_iterator it = map.find(topic);
      if (it != map.end())
      {
        for (const auto &msgEntry : it->second)
        {
          rowIDs.push_back(msgEntry.second);
        }
      }
    }

    SqlStatement sql = QueryOptions::StandardMessageQueryPreamble();
    sql.statement += " WHERE (";
    AppendTopicListClause(sql, rowIDs);
    sql.statement += ")";

    return sql;
  }

  /// \brief Topics for this option
  public: std::set<std::string> topics;
};

//////////////////////////////////////////////////
TopicList::TopicList(
    const std::set<std::string> &_topics,
    const QualifiedTimeRange &_timeRange)
  : TimeRangeOption(_timeRange),
    dataPtr(new Implementation{_topics})
{
  // Do nothing
}

//////////////////////////////////////////////////
TopicList::TopicList(
    const std::string &_singleTopic,
    const QualifiedTimeRange &_timeRange)
  : TopicList(std::set<std::string>{_singleTopic}, _timeRange)
{
  // Do nothing
}

//////////////////////////////////////////////////
TopicList::TopicList(const TopicList &_other)
  : TimeRangeOption(_other),
    dataPtr(new Implementation{*_other.dataPtr})
{
  // Do nothing
}

//////////////////////////////////////////////////
TopicList::TopicList(TopicList &&_other)  // NOLINT(build/c++11)
  : TimeRangeOption(std::move(_other)),
    dataPtr(std::move(_other.dataPtr))  // Is this legal?
{
  // Do nothing
}

//////////////////////////////////////////////////
std::set<std::string> &TopicList::Topics()
{
  return this->dataPtr->topics;
}

//////////////////////////////////////////////////
const std::set<std::string> &TopicList::Topics() const
{
  return this->dataPtr->topics;
}

//////////////////////////////////////////////////
std::vector<SqlStatement> TopicList::GenerateStatements(
    const Descriptor &_descriptor) const
{
  SqlStatement sql = this->dataPtr->GenerateStatement(_descriptor);

  // Add the time range condition
  const SqlStatement &timeCondition = this->GenerateTimeConditions();
  if (!timeCondition.statement.empty())
  {
    sql.statement += " AND (";
    sql.Append(timeCondition);
    sql.statement += ")";
  }

  sql.Append(QueryOptions::StandardMessageQueryClose());

  return {sql};
}

//////////////////////////////////////////////////
TopicList::~TopicList()
{
  // Destroy the pimpl
}

//////////////////////////////////////////////////
class TopicPattern::Implementation
{
  /// \brief Generate a WHERE clause for topics that match the requested pattern
  /// \param[in] _descriptor The descriptor forwarded by the interface class
  /// \return The desired WHERE clause
  public: SqlStatement GenerateStatement(
      const Descriptor &_descriptor)
  {
    const Descriptor::NameToMap &map = _descriptor.TopicsToMsgTypesToId();
    std::vector<int64_t> rowIDs;
    rowIDs.reserve(map.size());

    // Look through all the topics
    for (const auto &topicEntry : map)
    {
      // Find which topics match the pattern
      if (std::regex_match(topicEntry.first, this->pattern))
      {
        // Add all the rows of that topic to the list of IDs.
        for (const auto &msgEntry : topicEntry.second)
        {
          rowIDs.push_back(msgEntry.second);
        }
      }
    }

    SqlStatement sql = QueryOptions::StandardMessageQueryPreamble();
    sql.statement += " WHERE (";
    AppendTopicListClause(sql, rowIDs);
    sql.statement += ")";

    return sql;
  }

  /// \brief Pattern for this option
  /// TODO(anyone): Consider making this a vector of patterns?
  public: std::regex pattern;
};

//////////////////////////////////////////////////
TopicPattern::TopicPattern(
    const std::regex &_pattern,
    const QualifiedTimeRange &_timeRange)
  : TimeRangeOption(_timeRange),
    dataPtr(new Implementation{_pattern})
{
  // Do nothing
}

//////////////////////////////////////////////////
TopicPattern::TopicPattern(const TopicPattern &_other)
  : TimeRangeOption(_other),
    dataPtr(new Implementation{*_other.dataPtr})
{
  // Do nothing
}

//////////////////////////////////////////////////
TopicPattern::TopicPattern(TopicPattern &&_other)  // NOLINT(build/c++11)
  : TimeRangeOption(std::move(_other)),
    dataPtr(std::move(_other.dataPtr))
{
  // Do nothing
}

//////////////////////////////////////////////////
std::regex &TopicPattern::Pattern()
{
  return this->dataPtr->pattern;
}

//////////////////////////////////////////////////
const std::regex &TopicPattern::Pattern() const
{
  return this->dataPtr->pattern;
}

//////////////////////////////////////////////////
std::vector<SqlStatement> TopicPattern::GenerateStatements(
    const Descriptor &_descriptor) const
{
  SqlStatement sql = this->dataPtr->GenerateStatement(_descriptor);

  // Add the time range condition
  const SqlStatement &timeCondition = this->GenerateTimeConditions();
  if (!timeCondition.statement.empty())
  {
    sql.statement += " AND (";
    sql.Append(timeCondition);
    sql.statement += ")";
  }

  sql.Append(QueryOptions::StandardMessageQueryClose());

  return {sql};
}

//////////////////////////////////////////////////
TopicPattern::~TopicPattern()
{
  // Destroy the pimpl
}

//////////////////////////////////////////////////
class AllTopics::Implementation
{
  // Empty for now
};

//////////////////////////////////////////////////
AllTopics::AllTopics(const QualifiedTimeRange &_timeRange)
  : TimeRangeOption(_timeRange)
{
  // Do nothing. We don't even allocate the PIMPL because we have no use for it
  // yet. We might never need it, but we'll hang onto it just in case the needs
  // of this class change in the future.
}

//////////////////////////////////////////////////
AllTopics::AllTopics(const AllTopics &_other)
  : TimeRangeOption(_other)
{
  // Do nothing
}

//////////////////////////////////////////////////
AllTopics::AllTopics(AllTopics &&_other)  // NOLINT(build/c++11)
  : TimeRangeOption(std::move(_other))
{
  // Do nothing
}

//////////////////////////////////////////////////
std::vector<SqlStatement> AllTopics::GenerateStatements(
    const Descriptor & /*_descriptor*/) const
{
  SqlStatement sql = this->StandardMessageQueryPreamble();

  const SqlStatement &timeCondition = this->GenerateTimeConditions();
  if (!timeCondition.statement.empty())
  {
    sql.statement += "WHERE ";
    sql.Append(timeCondition);
  }

  sql.Append(this->StandardMessageQueryClose());

  return {sql};
}

//////////////////////////////////////////////////
AllTopics::~AllTopics()
{
  // Destroy the pimpl
}
