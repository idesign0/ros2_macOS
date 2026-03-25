/*
 * Copyright (C) 2024 Open Source Robotics Foundation
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

#include <yaml.h>
#include <cstdio>
#include <sstream>
#include <stack>
#include <string>

#include <gz/common/Console.hh>
#include <gz/common/Filesystem.hh>
#include <gz/common/Util.hh>

#include "gz/fuel_tools/ServerConfig.hh"

namespace gz::fuel_tools
{
//////////////////////////////////////////////////
/// \brief Private data class
class ServerConfigPrivate
{
  /// \brief Clear values.
  public: void Clear()
          {
            this->url.Clear();
            this->key = "";
            this->version = "1.0";
          }

  /// \brief URL to reach server
  public: common::URI url{"https://fuel.gazebosim.org", true};

  /// \brief A key to auth with the server
  public: std::string key = "";

  /// \brief The protocol version used when talking with this server.
  public: std::string version = "1.0";
};

//////////////////////////////////////////////////
ServerConfig::ServerConfig()
  : dataPtr (new ServerConfigPrivate)
{
}

//////////////////////////////////////////////////
ServerConfig::ServerConfig(const ServerConfig &_orig)
  : dataPtr(new ServerConfigPrivate)
{
  *(this->dataPtr) = *(_orig.dataPtr);
}

//////////////////////////////////////////////////
void ServerConfig::Clear()
{
  this->dataPtr->Clear();
}

//////////////////////////////////////////////////
ServerConfig &ServerConfig::operator=(const ServerConfig &_orig)
{
  *(this->dataPtr) = *(_orig.dataPtr);
  return *this;
}

//////////////////////////////////////////////////
ServerConfig::~ServerConfig()
{
}

//////////////////////////////////////////////////
common::URI ServerConfig::Url() const
{
  return this->dataPtr->url;
}

//////////////////////////////////////////////////
void ServerConfig::SetUrl(const common::URI &_url)
{
  this->dataPtr->url = _url;
}

//////////////////////////////////////////////////
std::string ServerConfig::ApiKey() const
{
  return this->dataPtr->key;
}

//////////////////////////////////////////////////
void ServerConfig::SetApiKey(const std::string &_key)
{
  this->dataPtr->key = _key;
}

//////////////////////////////////////////////////
std::string ServerConfig::Version() const
{
  return this->dataPtr->version;
}

//////////////////////////////////////////////////
void ServerConfig::SetVersion(const std::string &_version)
{
  this->dataPtr->version = _version;
}

//////////////////////////////////////////////////
std::string ServerConfig::AsString(const std::string &_prefix) const
{
  std::stringstream out;
  out << _prefix << "URL: " << this->Url().Str() << std::endl
      << _prefix << "Version: " << this->Version() << std::endl
      << _prefix << "API key: " << this->ApiKey() << std::endl;
  return out.str();
}

//////////////////////////////////////////////////
std::string ServerConfig::AsPrettyString(const std::string &_prefix) const
{
  std::string prop = "\033[96m\033[1m";
  std::string value = "\033[37m";
  std::string reset = "\033[0m";

  std::stringstream out;

  if (this->Url().Valid())
  {
    out << _prefix << prop << "URL: " << reset
        << value << this->Url().Str() << reset << std::endl;
  }

  if (!this->Version().empty())
  {
    out << _prefix << prop << "Version: " << reset
        << value << this->Version() << reset << std::endl;
  }

  if (!this->ApiKey().empty())
  {
    out << _prefix << prop << "API key: " << reset
        << value << this->ApiKey() << reset << std::endl;
  }
  return out.str();
}
}  // namespace gz::fuel_tools
