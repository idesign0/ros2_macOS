/*
 * Copyright (C) 2014 Open Source Robotics Foundation
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

#include <string>

#include "gz/transport/TopicUtils.hh"

namespace gz::transport
{
//////////////////////////////////////////////////
bool TopicUtils::IsValidNamespace(const std::string &_ns)
{
  // An empty namespace is valid, so take a shortcut here.
  if (_ns.empty())
    return true;

  // Too long string is not valid.
  if (_ns.size() > kMaxNameLength)
    return false;

  // "/" is not valid.
  if (_ns == "/")
    return false;

  // If the topic name has a '~' is not valid.
  if (_ns.find("~") != std::string::npos)
    return false;

  // If the topic name has a white space is not valid.
  if (_ns.find(" ") != std::string::npos)
    return false;

  // It is not allowed to have two consecutive slashes.
  if (_ns.find("//") != std::string::npos)
    return false;

  // It the topic name has a '@' is not valid.
  if (_ns.find("@") != std::string::npos)
    return false;

  // If the topic name has a ':=' is not valid.
  if (_ns.find(":=") != std::string::npos)
    return false;

  return true;
}

//////////////////////////////////////////////////
bool TopicUtils::IsValidPartition(const std::string &_partition)
{
  // A valid namespace is also a valid partition.
  return IsValidNamespace(_partition);
}

//////////////////////////////////////////////////
bool TopicUtils::IsValidTopic(const std::string &_topic)
{
  return IsValidNamespace(_topic) && !_topic.empty();
}

//////////////////////////////////////////////////
bool TopicUtils::FullyQualifiedName(const std::string &_partition,
  const std::string &_ns, const std::string &_topic, std::string &_name)
{
  // Sanity check, first things first.
  if (!IsValidPartition(_partition) || !IsValidNamespace(_ns) ||
      !IsValidTopic(_topic))
  {
    return false;
  }

  std::string partition = _partition;
  std::string ns = _ns;
  std::string topic = _topic;

  // If partition is not empty and does not start with slash, add it.
  if (!partition.empty() && partition.front() != '/')
    partition.insert(0, 1, '/');

  // If the partition contains a trailing slash, remove it.
  if (!partition.empty() && partition.back() == '/')
    partition.pop_back();

  // If the namespace does not contain a trailing slash, append it.
  if (ns.empty() || ns.back() != '/')
    ns.push_back('/');

  // If the namespace does not start with slash, add it.
  if (ns.empty() || ns.front() != '/')
    ns.insert(0, 1, '/');

  // If the topic ends in "/", remove it.
  if (!topic.empty() && topic.back() == '/')
    topic.pop_back();

  // If the topic does starts with '/' is considered an absolute topic and the
  // namespace will not be prefixed.
  if (!topic.empty() && topic.front() == '/')
    _name = topic;
  else
    _name = ns + topic;

  // Add the partition prefix.
  _name.insert(0, "@" + partition + "@");

  // Too long string is not valid.
  if (_name.size() > kMaxNameLength)
    return false;

  return true;
}

//////////////////////////////////////////////////
bool TopicUtils::DecomposeFullyQualifiedTopic(
    const std::string &_fullyQualifiedName,
    std::string &_partition,
    std::string &_namespaceAndTopic)
{
  const std::size_t firstAt = _fullyQualifiedName.find_first_of("@");
  const std::size_t lastAt = _fullyQualifiedName.find_last_of("@");

  if ( firstAt != 0
    || firstAt == lastAt
    || lastAt == _fullyQualifiedName.size() - 1)
  {
    return false;
  }

  std::string possiblePartition = _fullyQualifiedName.substr(
    firstAt + 1, lastAt - firstAt - 1);
  std::string possibleTopic = _fullyQualifiedName.substr(lastAt + 1);

  if (!IsValidPartition(possiblePartition) || !IsValidTopic(possibleTopic))
  {
    return false;
  }

  _partition = possiblePartition;
  _namespaceAndTopic = possibleTopic;
  return true;
}

//////////////////////////////////////////////////
std::string TopicUtils::AsValidTopic(const std::string &_topic)
{
  std::string validTopic;
  validTopic.reserve(_topic.size());

  for (std::size_t i = 0; i < _topic.size(); ++i)
  {
    char c = _topic[i];

    // Substitute spaces with '_'.
    if (c == ' ')
    {
      validTopic += '_';
      continue;
    }

    // Skip '@' and '~'.
    if (c == '@' || c == '~')
      continue;

    // Skip ":=".
    if (c == ':' && i + 1 < _topic.size() && _topic[i + 1] == '=')
    {
      ++i;
      continue;
    }

    // Skip "//".
    if (c == '/' && i + 1 < _topic.size() && _topic[i + 1] == '/')
    {
      ++i;
      continue;
    }

    validTopic += c;
  }

  if (!IsValidTopic(validTopic))
    return std::string();

  return validTopic;
}
}  // namespace gz::transport
