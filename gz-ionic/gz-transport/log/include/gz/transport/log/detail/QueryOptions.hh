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

#ifndef GZ_TRANSPORT_LOG_DETAIL_QUERYOPTIONS_HH_
#define GZ_TRANSPORT_LOG_DETAIL_QUERYOPTIONS_HH_

#include <set>
#include <string>

#include <gz/transport/config.hh>
#include <gz/transport/log/QueryOptions.hh>

namespace gz::transport::log
{
  // Inline bracket to help doxygen filtering.
  inline namespace GZ_TRANSPORT_VERSION_NAMESPACE {
  //
  //////////////////////////////////////////////////
  template <typename Container>
  TopicList TopicList::Create(
      const Container &_topics,
      const QualifiedTimeRange &_timeRange)
  {
    TopicList tl(std::set<std::string>(), _timeRange);

    std::set<std::string> &internalTopics = tl.Topics();
    for (const std::string &topic : _topics)
      internalTopics.insert(topic);

    return tl;
  }
  }
}

#endif
