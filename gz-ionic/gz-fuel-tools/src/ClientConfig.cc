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

#include <yaml.h>
#include <cstdio>
#include <sstream>
#include <stack>
#include <string>
#include <vector>
#include <gz/common/Console.hh>
#include <gz/common/Filesystem.hh>
#include <gz/common/Util.hh>

#include "gz/fuel_tools/ClientConfig.hh"
#include "gz/fuel_tools/config.hh"

namespace gz::fuel_tools
{
//////////////////////////////////////////////////
/// \brief Private data class
class ClientConfigPrivate
{
  /// \brief Constructor.
  public: ClientConfigPrivate()
          {
            std::string homePath;
            gz::common::env(GZ_HOMEDIR, homePath);
            this->cacheLocation = common::joinPaths(
                homePath, ".gz", "fuel");

            this->servers.push_back(ServerConfig());
          }

  /// \brief Clear values.
  public: void Clear()
          {
            this->servers.clear();
            this->cacheLocation = "";
            this->configPath = "";
            this->userAgent =
              "GazeboFuelTools-" GZ_FUEL_TOOLS_VERSION_FULL;
          }

  /// \brief A list of servers.
  public: std::vector<ServerConfig> servers;

  /// \brief a path on disk to where data is cached.
  public: std::string cacheLocation = "";

  /// \brief The path where the configuration file is located.
  public: std::string configPath = "";

  /// \brief Name of the user agent.
  public: std::string userAgent =
          "GazeboFuelTools-" GZ_FUEL_TOOLS_VERSION_FULL;
};

//////////////////////////////////////////////////
ClientConfig::ClientConfig() : dataPtr(new ClientConfigPrivate)
{
  std::string gzFuelPath = "";
  if (!gz::common::env("GZ_FUEL_CACHE_PATH", gzFuelPath))
  {
    return;
  }

  if (!gzFuelPath.empty())
  {
    if (!gz::common::isDirectory(gzFuelPath))
      gzerr << "[" << gzFuelPath << "] is not a directory" << std::endl;
    else
      this->SetCacheLocation(gzFuelPath);
  }

  std::string configYamlFile = common::joinPaths(this->CacheLocation(),
                                                 "config.yaml");
  std::string configYmlFile = common::joinPaths(this->CacheLocation(),
                                                "config.yml");
  if (gz::common::exists(configYamlFile))
    this->LoadConfig(configYamlFile);
  else if (gz::common::exists(configYmlFile))
    this->LoadConfig(configYmlFile);
}

//////////////////////////////////////////////////
ClientConfig::ClientConfig(const ClientConfig &_copy)
  : dataPtr(new ClientConfigPrivate)
{
  *(this->dataPtr) = *(_copy.dataPtr);
}

//////////////////////////////////////////////////
ClientConfig &ClientConfig::operator=(const ClientConfig &_copy)
{
  *(this->dataPtr) = *(_copy.dataPtr);

  return *this;
}

//////////////////////////////////////////////////
ClientConfig::~ClientConfig()
{
}

//////////////////////////////////////////////////
void ClientConfig::Clear()
{
  this->dataPtr->Clear();
}

//////////////////////////////////////////////////
bool ClientConfig::LoadConfig(const std::string &_file)
{
  // Sanity check: Verify that the configuration file exists.
  if (!gz::common::exists(_file))
  {
    gzerr << "Unable to find configuration file [" << _file<< "]" << std::endl;
    return false;
  }


  FILE *fh = fopen(_file.c_str(), "r");
  if (!fh)
  {
    gzerr << "Failed to open configuration file ["
      << _file << "]" << std::endl;
    return false;
  }

  this->dataPtr->configPath = _file;

  // Initialize parser.
  yaml_parser_t parser;
  if (!yaml_parser_initialize(&parser))
  {
    gzerr << "Failed to initialize YAML parser" << std::endl;
    fclose(fh);
    return false;
  }

  // Set input file.
  yaml_parser_set_input_file(&parser, fh);

  bool res = true;
  yaml_event_t event;
  std::stack<std::string> tokens;
  tokens.push("root");
  std::string serverURL = "";
  std::string cacheLocationConfig = "";
  std::string privateToken = "";

  do
  {
    if (!yaml_parser_parse(&parser, &event))
    {
      gzerr << "Parser error [" << parser.error << "]" << std::endl;
      res = false;
      break;
    }

    switch (event.type)
    {
      case YAML_NO_EVENT:
      // Stream start/end.
      case YAML_STREAM_START_EVENT:
      case YAML_STREAM_END_EVENT:
      // Block delimiters.
      case YAML_DOCUMENT_START_EVENT:
      case YAML_DOCUMENT_END_EVENT:
      case YAML_SEQUENCE_START_EVENT:
        break;
      case YAML_SEQUENCE_END_EVENT:
        if (!tokens.empty())
          tokens.pop();
        break;
      case YAML_MAPPING_START_EVENT:
        if (!tokens.empty() && tokens.top() == "servers")
        {
          tokens.push("server");
          serverURL = "";
        }
        break;
      case YAML_MAPPING_END_EVENT:
        if (!tokens.empty() && tokens.top() == "cache")
        {
          if (cacheLocationConfig.empty())
          {
            gzerr << "[path] parameter is required for a cache" << std::endl;
            res = false;
          }
        }
        else if (!tokens.empty() && tokens.top() == "server")
        {
          if (!serverURL.empty())
          {
            // Sanity check: Make sure that the server is not already stored.
            bool repeated = false;
            for (auto & savedServer : this->MutableServers())
            {
              if (savedServer.Url().Str() == serverURL)
              {
                if (!privateToken.empty())
                {
                  gzmsg << "Set private token for " << serverURL << " server."
                    << std::endl;
                  savedServer.SetApiKey(privateToken);
                }
                gzwarn << "URL [" << serverURL << "] already exists. "
                  << "Ignoring server" << std::endl;
                repeated = true;
                break;
              }
            }
            if (!repeated)
            {
              // Add the new server.
              ServerConfig newServer;
              newServer.SetUrl(common::URI(serverURL));
              if (!privateToken.empty())
              {
                gzmsg << "Set private token for " << serverURL << " server."
                  << std::endl;
                newServer.SetApiKey(privateToken);
              }
              this->AddServer(newServer);
            }
          }
          else
          {
            gzerr << "[url] parameter is required for a server"
                      << std::endl;
            res = false;
          }
        }

        if (!tokens.empty())
          tokens.pop();
        break;
      // Data .
      case YAML_ALIAS_EVENT:
        break;
      case YAML_SCALAR_EVENT:
        if (!tokens.empty() && tokens.top() == "url")
        {
          std::string url(
            reinterpret_cast<const char *>(event.data.scalar.value));
          serverURL = url;
          tokens.pop();
        }
        else if (!tokens.empty() && tokens.top() == "path")
        {
          std::string path(
            reinterpret_cast<const char *>(event.data.scalar.value));
          cacheLocationConfig = path;
          tokens.pop();
        }
        else if (!tokens.empty() && tokens.top() == "private-token")
        {
          std::string token(
            reinterpret_cast<const char *>(event.data.scalar.value));
          privateToken = token;
          tokens.pop();
        }
        else
        {
          std::string key(
            reinterpret_cast<const char *>(event.data.scalar.value));
          if (!key.empty())
            tokens.push(key);
        }

        break;
      default:
      {
        gzerr << "Unknown event type [" << event.type << "]" << std::endl;
        res = false;
      }
    }

    if (event.type != YAML_STREAM_END_EVENT)
      yaml_event_delete(&event);
  } while (event.type != YAML_STREAM_END_EVENT);

  // Default cache path.
  std::string homePath;
  gz::common::env(GZ_HOMEDIR, homePath);
  std::string cacheLocation = gz::common::joinPaths(
    homePath, ".gz", "fuel");

  // The user wants to overwrite the default cache path.
  if (!cacheLocationConfig.empty())
    cacheLocation = cacheLocationConfig;

  // Do not overwrite the cache location if GZ_FUEL_CACHE_PATH is set.
  std::string gzFuelPath = "";
  if (gz::common::env("GZ_FUEL_CACHE_PATH", gzFuelPath))
  {
    gzwarn << "GZ_FUEL_CACHE_PATH is set to [" << gzFuelPath << "]. The "
            << "path in the configuration file will be ignored" << std::endl;
    cacheLocation = gzFuelPath;
  }
  this->SetCacheLocation(cacheLocation);

  // Cleanup.
  yaml_parser_delete(&parser);
  fclose(fh);

  return res;
}

//////////////////////////////////////////////////
std::string ClientConfig::ConfigPath() const
{
  return this->dataPtr->configPath;
}

//////////////////////////////////////////////////
std::vector<ServerConfig> ClientConfig::Servers() const
{
  return this->dataPtr->servers;
}

//////////////////////////////////////////////////
std::vector<ServerConfig> & ClientConfig::MutableServers() const
{
  return this->dataPtr->servers;
}

//////////////////////////////////////////////////
void ClientConfig::AddServer(const ServerConfig &_srv)
{
  this->dataPtr->servers.push_back(_srv);
}

//////////////////////////////////////////////////
std::string ClientConfig::CacheLocation() const
{
  return this->dataPtr->cacheLocation;
}

//////////////////////////////////////////////////
void ClientConfig::SetCacheLocation(const std::string &_path)
{
  this->dataPtr->cacheLocation = _path;
}

//////////////////////////////////////////////////
void ClientConfig::SetUserAgent(const std::string &_agent)
{
  this->dataPtr->userAgent = _agent;
}

//////////////////////////////////////////////////
const std::string &ClientConfig::UserAgent() const
{
  return this->dataPtr->userAgent;
}

//////////////////////////////////////////////////
std::string ClientConfig::AsString(const std::string &_prefix) const
{
  std::stringstream out;
  out << _prefix << "Config path: " << this->ConfigPath() << std::endl
      << _prefix << "Cache location: " << this->CacheLocation() << std::endl
      << _prefix << "Servers:" << std::endl;

  for (const auto &s : this->Servers())
  {
    out << _prefix << "  ---" << std::endl;
    out << _prefix << s.AsString("  ");
  }

  return out.str();
}
}  // namespace gz::fuel_tools
