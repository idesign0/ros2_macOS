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

#ifndef GZ_FUEL_TOOLS_JSONPARSER_HH_
#define GZ_FUEL_TOOLS_JSONPARSER_HH_

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "gz/fuel_tools/ModelIdentifier.hh"
#include "gz/fuel_tools/ModelIter.hh"
#include "gz/fuel_tools/WorldIdentifier.hh"
#include "gz/fuel_tools/WorldIter.hh"

namespace Json
{
  class Value;
}

namespace gz::fuel_tools
{
  class ServerConfig;

  /// \brief A helper class for making REST requests.
  class GZ_FUEL_TOOLS_VISIBLE JSONParser
  {
    /// \brief Parse a model JSON string and return a model identifier
    /// \param[in] _json JSON string containing a model.
    /// \param[in] _server The server sending the JSON.
    /// This information is populated into some fields of the model.
    /// \return a model identifier
    public: static ModelIdentifier ParseModel(
                const std::string &_json,
                const ServerConfig &_server);

    /// \brief Parse a world JSON string and return a world identifier
    /// \param[in] _json JSON string containing a world.
    /// \param[in] _server The server sending the JSON.
    /// This information is populated into some fields of the world.
    /// \return A world identifier
    public: static WorldIdentifier ParseWorld(
                const std::string &_json,
                const ServerConfig &_server);

    /// \brief Parse a model array JSON string and return a model iterator
    /// \param[in] _json JSON string containing an array of models
    /// \param[in] _server The server sending the JSON.
    /// This information is populated into some fields of the models.
    /// \return vector of model identifiers
    public: static std::vector<ModelIdentifier> ParseModels(
                const std::string &_json,
                const ServerConfig &_server);

    /// \brief Parse a world array JSON string and return a world iterator
    /// \param[in] _json JSON string containing an array of worlds
    /// \param[in] _server The server sending the JSON.
    /// This information is populated into some fields of the worlds.
    /// \return vector of world identifiers
    public: static std::vector<WorldIdentifier> ParseWorlds(
                const std::string &_json,
                const ServerConfig &_server);

    /// \brief Build a model iterator from a JSON string
    /// \param[in] _modelIt A model iterator containing only one model
    /// \return A JSON string representing a single model
    public: static std::string BuildModel(ModelIter _modelIt);

    /// \brief Build a world iterator from a JSON string
    /// \param[in] _worldIt A world iterator containing only one world
    /// \return A JSON string representing a single world
    public: static std::string BuildWorld(WorldIter _worldIt);

    /// \brief Parse a license array JSON string and return a map of
    /// licenses.
    /// \param[in] _json JSON string containing an array of models
    /// \param[out] _licenses License information where the keys are license
    /// names and values are license id's on the Fuel server.
    /// \return True on success, false otherwise.
    public: static bool ParseLicenses(const std::string &_json,
                std::map<std::string, unsigned int> &_licenses);

    /// \brief Parse a json object as a model.
    /// \param[in] _json JSON object containing a single model
    /// \param[out] _model a model identifier after parsing the JSON
    /// \return True if the parsing succeed or false otherwise
    private: static bool ParseModelImpl(
                const Json::Value &_json, ModelIdentifier &_model);

    /// \brief Parse a JSON object as a world.
    /// \param[in] _json JSON object containing a single world
    /// \param[out] _world a world identifier after parsing the JSON
    /// \return True if the parsing succeed or false otherwise
    private: static bool ParseWorldImpl(
                const Json::Value &_json, WorldIdentifier &_world);

    /// \brief Parse the list of tags contained in a model.
    /// \param[in] _json JSON representation of the model.
    /// \return The list of tags.
    private: static std::vector<std::string> ParseTags(
                const Json::Value &_json);
  };
}  // namespace gz::fuel_tools

#endif  // GZ_FUEL_TOOLS_JSONPARSER_HH_
