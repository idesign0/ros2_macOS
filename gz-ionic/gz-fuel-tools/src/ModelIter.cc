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

#include <memory>
#include <regex>
#include <string>
#include <vector>
#include <gz/common/Console.hh>

#include "gz/fuel_tools/ClientConfig.hh"
#include "gz/fuel_tools/JSONParser.hh"
#include "gz/fuel_tools/Model.hh"
#include "gz/fuel_tools/ModelIdentifier.hh"
#include "gz/fuel_tools/ModelIter.hh"
#include "gz/fuel_tools/RestClient.hh"

#include "ModelIterPrivate.hh"
#include "ModelPrivate.hh"

namespace gz::fuel_tools
{
//////////////////////////////////////////////////
ModelIter ModelIterFactory::Create(const std::vector<ModelIdentifier> &_ids)
{
  std::unique_ptr<ModelIterPrivate> priv(new IterIds(_ids));
  return ModelIter(std::move(priv));
}

//////////////////////////////////////////////////
ModelIter ModelIterFactory::Create(const std::vector<Model> &_models)
{
  std::unique_ptr<ModelIterPrivate> priv(new IterModels(_models));
  return ModelIter(std::move(priv));
}

//////////////////////////////////////////////////
ModelIter ModelIterFactory::Create(const Rest &_rest,
    const ServerConfig &_server, const std::string &_api)
{
  std::unique_ptr<ModelIterPrivate> priv(new IterRestIds(
    _rest, _server, _api));
  return ModelIter(std::move(priv));
}

//////////////////////////////////////////////////
ModelIter ModelIterFactory::Create()
{
  std::unique_ptr<ModelIterPrivate> priv(new IterIds({}));
  return ModelIter(std::move(priv));
}

//////////////////////////////////////////////////
ModelIterPrivate::~ModelIterPrivate()
{
}

//////////////////////////////////////////////////
IterIds::~IterIds()
{
}

//////////////////////////////////////////////////
IterIds::IterIds(std::vector<ModelIdentifier> _ids)
  : ids(_ids)
{
  this->idIter = this->ids.begin();
  if (!this->ids.empty())
  {
    std::shared_ptr<ModelPrivate> ptr(new ModelPrivate);
    ptr->id = *(this->idIter);
    this->model = Model(ptr);
  }
}

//////////////////////////////////////////////////
void IterIds::Next()
{
  // advance pointer
  ++(this->idIter);

  // Update personal model class
  if (this->idIter != this->ids.end())
  {
    std::shared_ptr<ModelPrivate> ptr(new ModelPrivate);
    ptr->id = *(this->idIter);
    this->model = Model(ptr);
  }
}

//////////////////////////////////////////////////
bool IterIds::HasReachedEnd()
{
  return this->ids.empty() || this->idIter == this->ids.end();
}

//////////////////////////////////////////////////
IterModels::~IterModels()
{
}

//////////////////////////////////////////////////
IterModels::IterModels(std::vector<Model> _models)
  : models(_models)
{
  this->modelIter = this->models.begin();
  if (!this->models.empty())
  {
    this->model = this->models.front();
  }
}

//////////////////////////////////////////////////
void IterModels::Next()
{
  // advance pointer
  ++(this->modelIter);

  // Update personal model class
  if (this->modelIter != this->models.end())
  {
    this->model = *(this->modelIter);
  }
}

//////////////////////////////////////////////////
bool IterModels::HasReachedEnd()
{
  return this->models.empty() || this->modelIter == this->models.end();
}

//////////////////////////////////////////////////
IterRestIds::~IterRestIds()
{
}

std::vector<ModelIdentifier> IterRestIds::ParseIdsFromResponse(
    const RestResponse &resp)
{
  if (resp.data == "null\n" || resp.statusCode != 200)
    return {};

  // Parse the response.
  return JSONParser::ParseModels(resp.data, this->config);
}

//////////////////////////////////////////////////
IterRestIds::IterRestIds(const Rest &_rest, const ServerConfig &_config,
    const std::string &_api)
  : config(_config), rest(_rest), api(_api)
{
  this->idIter = this->ids.begin();
  this->Next();
}

//////////////////////////////////////////////////
RestResponse IterRestIds::MakeRestRequest(std::size_t _page)
{
  HttpMethod method = HttpMethod::GET;
  std::vector<std::string> headers = {"Accept: application/json"};
  std::vector<ModelIdentifier> modelIds;
  // Prepare the request with the requested page.
  std::string queryStrPage = "page=" + std::to_string(_page);
  std::string path = this->api;
  // Fire the request.
  return this->rest.Request(method, this->config.Url().Str(),
      this->config.Version(),
      std::regex_replace(path, std::regex(R"(\\)"), "/"),
      {queryStrPage}, headers, "");
}

//////////////////////////////////////////////////
void IterRestIds::Next()
{
  // advance pointer
  if (this->idIter != this->ids.end())
    ++(this->idIter);

  if (this->idIter == this->ids.end())
  {
    ++this->currentPage;
    RestResponse resp = this->MakeRestRequest(this->currentPage);
    this->ids = this->ParseIdsFromResponse(resp);
    this->idIter = this->ids.begin();
  }

  // Update personal model class
  if (this->idIter != this->ids.end())
  {
    std::shared_ptr<ModelPrivate> ptr(new ModelPrivate);
    ptr->id = *(this->idIter);
    ptr->id.SetServer(this->config);
    this->model = Model(ptr);
  }
}

//////////////////////////////////////////////////
bool IterRestIds::HasReachedEnd()
{
  return this->ids.empty() || this->idIter == this->ids.end();
}

//////////////////////////////////////////////////
ModelIter::ModelIter(std::unique_ptr<ModelIterPrivate> _dptr)
{
  this->dataPtr.reset(_dptr.release());
}

//////////////////////////////////////////////////
ModelIter::ModelIter(ModelIter && _old)
{
  this->dataPtr.reset(_old.dataPtr.release());
}

//////////////////////////////////////////////////
ModelIter::~ModelIter()
{
}

//////////////////////////////////////////////////
ModelIter::operator bool()
{
  return !this->dataPtr->HasReachedEnd();
}

//////////////////////////////////////////////////
ModelIter::operator bool() const
{
  return !this->dataPtr->HasReachedEnd();
}

//////////////////////////////////////////////////
ModelIter &ModelIter::operator++()
{
  if (!this->dataPtr->HasReachedEnd())
  {
    this->dataPtr->Next();
  }
  return *this;
}

//////////////////////////////////////////////////
Model &ModelIter::operator*()
{
  return this->dataPtr->model;
}

//////////////////////////////////////////////////
Model *ModelIter::operator->()
{
  return &(this->dataPtr->model);
}
}  // namespace gz::fuel_tools
