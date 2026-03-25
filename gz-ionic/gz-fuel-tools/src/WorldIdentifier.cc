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

#include <sstream>
#include <string>
#include <vector>

#include <gz/common/Filesystem.hh>
#include <gz/common/StringUtils.hh>
#include <gz/common/URI.hh>
#include <gz/common/Util.hh>

#include "gz/fuel_tools/ClientConfig.hh"
#include "gz/fuel_tools/Helpers.hh"
#include "gz/fuel_tools/WorldIdentifier.hh"


namespace gz::fuel_tools
{
class WorldIdentifierPrivate
{
  /// \brief a name given to this world by a user
  public: std::string name;

  /// \brief owner who this world is attributed to
  public: std::string owner;

  /// \brief Server of this world
  public: ServerConfig server;

  /// \brief World version. Valid versions start from 1, 0 means the tip.
  public: unsigned int version{0};

  /// \brief Path of this world in the local cache
  public: std::string localPath;

  /// \brief True indicates the world is private, false indicates the
  /// world is public.
  public: bool privacy{false};
};

//////////////////////////////////////////////////
WorldIdentifier::WorldIdentifier()
  : dataPtr(new WorldIdentifierPrivate)
{
}

//////////////////////////////////////////////////
WorldIdentifier::WorldIdentifier(const WorldIdentifier &_orig)
{
  this->dataPtr.reset(new WorldIdentifierPrivate(*(_orig.dataPtr.get())));
}

//////////////////////////////////////////////////
WorldIdentifier &WorldIdentifier::operator=(const WorldIdentifier &_orig)
{
  this->dataPtr.reset(new WorldIdentifierPrivate(*(_orig.dataPtr.get())));
  return *this;
}

//////////////////////////////////////////////////
bool WorldIdentifier::operator==(const WorldIdentifier &_rhs) const
{
  return this->UniqueName() == _rhs.UniqueName();
}

//////////////////////////////////////////////////
WorldIdentifier::~WorldIdentifier()
{
}

//////////////////////////////////////////////////
std::string WorldIdentifier::UniqueName() const
{
  return common::copyToUnixPath(common::joinPaths(
    uriToPath(this->dataPtr->server.Url()),
    this->dataPtr->owner, "worlds",
    this->dataPtr->name));
}

//////////////////////////////////////////////////
gz::common::URI WorldIdentifier::Url() const
{
  return common::URI(
    common::joinPaths(this->dataPtr->server.Url().Str(),
                      this->dataPtr->owner,
                      "worlds",
                      this->dataPtr->name), true);
}

//////////////////////////////////////////////////
std::string WorldIdentifier::Name() const
{
  return this->dataPtr->name;
}

//////////////////////////////////////////////////
bool WorldIdentifier::SetName(const std::string &_name)
{
  this->dataPtr->name = common::lowercase(_name);
  return true;
}

//////////////////////////////////////////////////
std::string WorldIdentifier::Owner() const
{
  return this->dataPtr->owner;
}

//////////////////////////////////////////////////
bool WorldIdentifier::SetOwner(const std::string &_name)
{
  this->dataPtr->owner = common::lowercase(_name);
  return true;
}

//////////////////////////////////////////////////
bool WorldIdentifier::SetServer(const ServerConfig &_server)
{
  bool success = _server.Url().Valid();
  if (success)
    this->dataPtr->server = _server;

  return success;
}

//////////////////////////////////////////////////
ServerConfig &WorldIdentifier::Server() const
{
  return this->dataPtr->server;
}

//////////////////////////////////////////////////
unsigned int WorldIdentifier::Version() const
{
  return this->dataPtr->version;
}

//////////////////////////////////////////////////
std::string WorldIdentifier::VersionStr() const
{
  std::string version = this->dataPtr->version == 0 ?
      "tip" : std::to_string(this->dataPtr->version);
  return version;
}

//////////////////////////////////////////////////
bool WorldIdentifier::SetVersion(const unsigned int _version)
{
  this->dataPtr->version = _version;
  return true;
}

//////////////////////////////////////////////////
bool WorldIdentifier::SetVersionStr(const std::string &_version)
{
  if (_version == "tip" || _version.empty())
  {
    this->dataPtr->version = 0;
    return true;
  }

  try
  {
    this->dataPtr->version = std::stoi(_version);
  }
  catch(std::invalid_argument &_e)
  {
    return false;
  }

  return true;
}

//////////////////////////////////////////////////
std::string WorldIdentifier::LocalPath() const
{
  return this->dataPtr->localPath;
}

//////////////////////////////////////////////////
bool WorldIdentifier::SetLocalPath(const std::string &_path)
{
  this->dataPtr->localPath = _path;
  return true;
}

//////////////////////////////////////////////////
std::string WorldIdentifier::AsString(const std::string &_prefix) const
{
  std::stringstream out;
  out << _prefix << "Name: " << this->Name() << std::endl
      << _prefix << "Owner: " << this->Owner() << std::endl
      << _prefix << "Version: " << this->VersionStr() << std::endl
      << _prefix << "Unique name: " << this->UniqueName() << std::endl
      << _prefix << "Local path: " << this->LocalPath() << std::endl
      << _prefix << "Server:" << std::endl
      << this->Server().AsString(_prefix + "  ");
  return out.str();
}

//////////////////////////////////////////////////
std::string WorldIdentifier::AsPrettyString(const std::string &_prefix) const
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

  if (this->Version() != 0)
  {
    out << _prefix << prop << "Version: " << reset
        << value << this->VersionStr() << reset << std::endl;
  }

  out << _prefix << prop << "Server:" << reset << std::endl
      << this->Server().AsPrettyString(_prefix + "  ");
  return out.str();
}
//////////////////////////////////////////////////
bool WorldIdentifier::Private() const
{
  return this->dataPtr->privacy;
}

//////////////////////////////////////////////////
void WorldIdentifier::SetPrivate(bool _private)
{
  this->dataPtr->privacy = _private;
}
}  // namespace gz::fuel_tools
