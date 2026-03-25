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

#include <memory>
#include <regex>
#include <string>
#include <vector>
#include <gz/common/Console.hh>

#include "gz/fuel_tools/ClientConfig.hh"
#include "gz/fuel_tools/JSONParser.hh"
#include "gz/fuel_tools/WorldIdentifier.hh"
#include "gz/fuel_tools/WorldIter.hh"
#include "gz/fuel_tools/RestClient.hh"

#include "WorldIterPrivate.hh"

namespace gz::fuel_tools
{

//////////////////////////////////////////////////
WorldIter WorldIterFactory::Create(const std::vector<WorldIdentifier> &_ids)
{
  std::unique_ptr<WorldIterPrivate> priv(new WorldIterIds(_ids));
  return WorldIter(std::move(priv));
}

//////////////////////////////////////////////////
WorldIter WorldIterFactory::Create(const Rest &_rest,
    const ServerConfig &_server, const std::string &_path)
{
  std::unique_ptr<WorldIterPrivate> priv(new WorldIterRestIds(
    _rest, _server, _path));
  return WorldIter(std::move(priv));
}

//////////////////////////////////////////////////
WorldIter WorldIterFactory::Create()
{
  std::unique_ptr<WorldIterPrivate> priv(new WorldIterIds({}));
  return WorldIter(std::move(priv));
}

//////////////////////////////////////////////////
WorldIterPrivate::~WorldIterPrivate()
{
}

//////////////////////////////////////////////////
WorldIterIds::~WorldIterIds()
{
}

//////////////////////////////////////////////////
WorldIterIds::WorldIterIds(std::vector<WorldIdentifier> _ids)
  : ids(_ids)
{
  this->idIter = this->ids.begin();
  if (!this->ids.empty())
  {
    this->worldId = *(this->idIter);
  }
}

//////////////////////////////////////////////////
void WorldIterIds::Next()
{
  // advance pointer
  ++(this->idIter);

  // Update personal world class
  if (this->idIter != this->ids.end())
  {
    this->worldId = *(this->idIter);
  }
}

//////////////////////////////////////////////////
bool WorldIterIds::HasReachedEnd()
{
  return this->ids.empty() || this->idIter == this->ids.end();
}

//////////////////////////////////////////////////
WorldIterRestIds::~WorldIterRestIds()
{
}

//////////////////////////////////////////////////
WorldIterRestIds::WorldIterRestIds(const Rest &_rest,
    const ServerConfig &_config, const std::string &_path)
  : config(_config), rest(_rest)
{
  auto method = HttpMethod::GET;
  this->config = _config;
  std::vector<std::string> headers = {"Accept: application/json"};
  RestResponse resp;
  std::vector<WorldIdentifier> worldIds;
  this->ids.clear();

  // Empty query string will get the first page of worlds.
  std::string queryStrPage  = "";
  std::string queryStrPageKey = "page=";
  do
  {
    // Fire the request.
    resp = this->rest.Request(method, this->config.Url().Str(),
      this->config.Version(),
      std::regex_replace(_path, std::regex(R"(\\)"), "/"),
      {queryStrPage}, headers, "");

    // Reset the query string
    queryStrPage = "";

    // Get the next page from the headers.
    if (resp.headers.find("Link") != resp.headers.end())
    {
      std::vector<std::string> links = gz::common::split(
          resp.headers["Link"], ",");
      for (const auto &l : links)
      {
        // cppcheck-suppress useStlAlgorithm
        if (l.find("next") != std::string::npos) {
          auto start = l.find(queryStrPageKey);
          auto end = l.find(">", start+1);
          queryStrPage = l.substr(start, end-start);
          break;
        }
      }
    }

    // Fallsafe - break if response code is invalid
    if (resp.data == "null\n" || resp.statusCode != 200)
    {
      break;
    }

    // Parse the response.
    worldIds = JSONParser::ParseWorlds(resp.data, this->config);

    // Add the vector of worlds to the list.
    this->ids.insert(std::end(this->ids), std::begin(worldIds),
      std::end(worldIds));
  } while (!worldIds.empty() && !queryStrPage.empty());

  if (this->ids.empty())
    return;

  this->idIter = this->ids.begin();

  // make first world
  this->worldId = *(this->idIter);
  this->worldId.SetServer(this->config);

  gzdbg << "Got response [" << resp.data << "]\n";
}

//////////////////////////////////////////////////
void WorldIterRestIds::Next()
{
  // advance pointer
  ++(this->idIter);

  // Update personal world class
  if (this->idIter != this->ids.end())
  {
    this->worldId = *(this->idIter);
    this->worldId.SetServer(this->config);
  }
  // TODO(nkoenig) request next page if api is paginated here instead of in the
  // constructor
}

//////////////////////////////////////////////////
bool WorldIterRestIds::HasReachedEnd()
{
  return this->ids.empty() || this->idIter == this->ids.end();
}

//////////////////////////////////////////////////
WorldIter::WorldIter(std::unique_ptr<WorldIterPrivate> _dptr)
{
  this->dataPtr.reset(_dptr.release());
}

//////////////////////////////////////////////////
WorldIter::WorldIter(WorldIter && _old)
{
  this->dataPtr.reset(_old.dataPtr.release());
}

//////////////////////////////////////////////////
WorldIter::~WorldIter()
{
}

//////////////////////////////////////////////////
WorldIter::operator bool() const
{
  return !this->dataPtr->HasReachedEnd();
}

//////////////////////////////////////////////////
WorldIter &WorldIter::operator++()
{
  if (!this->dataPtr->HasReachedEnd())
  {
    this->dataPtr->Next();
  }
  return *this;
}

//////////////////////////////////////////////////
WorldIdentifier *WorldIter::operator->()
{
  return &(this->dataPtr->worldId);
}

//////////////////////////////////////////////////
WorldIdentifier &WorldIter::operator*()
{
  return this->dataPtr->worldId;
}

//////////////////////////////////////////////////
WorldIter::operator WorldIdentifier() const
{
  return this->dataPtr->worldId;
}
}  // namespace gz::fuel_tools
