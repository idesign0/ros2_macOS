/*
 * Copyright (C) 2020 Open Source Robotics Foundation
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

#include <sstream>
#include <string>
#include <vector>

#include <gz/common/Filesystem.hh>
#include <gz/common/URI.hh>

#include "gz/fuel_tools/ClientConfig.hh"
#include "gz/fuel_tools/CollectionIdentifier.hh"
#include "gz/fuel_tools/Helpers.hh"


namespace gz::fuel_tools
{
class CollectionIdentifierPrivate
{
  /// \brief a name given to this collection by a user
  public: std::string name;

  /// \brief owner who this collection is attributed to
  public: std::string owner;

  /// \brief Server of this collection
  public: ServerConfig server;
};

//////////////////////////////////////////////////
CollectionIdentifier::CollectionIdentifier()
  : dataPtr(std::make_unique<CollectionIdentifierPrivate>())
{
}

//////////////////////////////////////////////////
CollectionIdentifier::CollectionIdentifier(const CollectionIdentifier &_orig)
    : dataPtr(std::make_unique<CollectionIdentifierPrivate>(*_orig.dataPtr))
{
}

//////////////////////////////////////////////////
CollectionIdentifier &CollectionIdentifier::operator=(
    const CollectionIdentifier &_orig)
{
  this->dataPtr = std::make_unique<CollectionIdentifierPrivate>(*_orig.dataPtr);
  return *this;
}

//////////////////////////////////////////////////
bool CollectionIdentifier::operator==(const CollectionIdentifier &_rhs) const
{
  return this->UniqueName() == _rhs.UniqueName();
}

//////////////////////////////////////////////////
CollectionIdentifier::~CollectionIdentifier() = default;

//////////////////////////////////////////////////
std::string CollectionIdentifier::UniqueName() const
{
  return common::copyToUnixPath(common::joinPaths(
    uriToPath(this->dataPtr->server.Url()),
    this->dataPtr->owner, "collections",
    this->dataPtr->name));
}

//////////////////////////////////////////////////
gz::common::URI CollectionIdentifier::Url() const
{
  return common::URI(
    common::joinPaths(this->dataPtr->server.Url().Str(),
                      this->dataPtr->owner,
                      "collections",
                      this->dataPtr->name), true);
}

//////////////////////////////////////////////////
std::string CollectionIdentifier::Name() const
{
  return this->dataPtr->name;
}

//////////////////////////////////////////////////
bool CollectionIdentifier::SetName(const std::string &_name)
{
  this->dataPtr->name = _name;
  return true;
}

//////////////////////////////////////////////////
std::string CollectionIdentifier::Owner() const
{
  return this->dataPtr->owner;
}

//////////////////////////////////////////////////
bool CollectionIdentifier::SetOwner(const std::string &_name)
{
  this->dataPtr->owner = _name;
  return true;
}

//////////////////////////////////////////////////
bool CollectionIdentifier::SetServer(const ServerConfig &_server)
{
  bool success = _server.Url().Valid();
  if (success)
    this->dataPtr->server = _server;

  return success;
}

//////////////////////////////////////////////////
ServerConfig &CollectionIdentifier::Server() const
{
  return this->dataPtr->server;
}

//////////////////////////////////////////////////
std::string CollectionIdentifier::AsString(const std::string &_prefix) const
{
  std::stringstream out;
  out << _prefix << "Name: " << this->Name() << std::endl
      << _prefix << "Owner: " << this->Owner() << std::endl
      << _prefix << "Unique name: " << this->UniqueName() << std::endl
      << _prefix << "Server:" << std::endl
      << this->Server().AsString(_prefix + "  ");
  return out.str();
}

//////////////////////////////////////////////////
std::string CollectionIdentifier::AsPrettyString(
    const std::string &_prefix) const
{
  std::string prop = "\033[96m\033[1m";
  std::string value = "\033[37m";
  std::string reset = "\033[0m";

  std::stringstream out;

  if (!this->Name().empty())
  {
    out << _prefix << prop << "Name: " << reset
        << value << this->Name() << reset << std::endl;
  }

  if (!this->Owner().empty())
  {
    out << _prefix << prop << "Owner: " << reset
        << value << this->Owner() << reset << std::endl;
  }

  out << _prefix << prop << "Server:" << reset << std::endl
      << this->Server().AsPrettyString(_prefix + "  ");
  return out.str();
}
}  // namespace gz::fuel_tools
