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

#ifndef GZ_TRANSPORT_NODEOPTIONSPRIVATE_HH_
#define GZ_TRANSPORT_NODEOPTIONSPRIVATE_HH_

#include <map>
#include <string>

#include "gz/transport/config.hh"
#include "gz/transport/NetUtils.hh"

namespace gz::transport
{
  // Inline bracket to help doxygen filtering.
  inline namespace GZ_TRANSPORT_VERSION_NAMESPACE {
  //
  /// \internal
  /// \brief Private data for NodeOption class.
  class NodeOptionsPrivate
  {
    /// \brief Constructor.
    public: NodeOptionsPrivate() = default;

    /// \brief Destructor.
    public: virtual ~NodeOptionsPrivate() = default;

    /// \brief Namespace for this node.
    public: std::string ns;

    /// \brief Partition for this node.
    public: std::string partition = hostname() + ":" + username();

    /// \brief Table of remappings. The key is the original topic name and
    /// its value is the new topic name to be used instead.
    public: std::map<std::string, std::string> topicsRemap;
  };
  }
}
#endif  // GZ_TRANSPORT_NODEOPTIONSPRIVATE_HH_
