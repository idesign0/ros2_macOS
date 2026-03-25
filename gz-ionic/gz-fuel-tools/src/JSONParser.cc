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

#include <json/json.h>
#include <string>
#include <vector>
#include <gz/common/Console.hh>

#include "gz/fuel_tools/ClientConfig.hh"
#include "gz/fuel_tools/JSONParser.hh"
#include "gz/fuel_tools/WorldIter.hh"

#if defined(_WIN32) && !defined(timegm)
  #define timegm _mkgmtime
#endif

#define GZ_JSON_HAVE_EXCEPTIONS ((JSONCPP_VERSION_MAJOR < 1 && \
      JSONCPP_VERSION_MINOR < 10) ||  JSON_USE_EXCEPTION == 0)

namespace gz::fuel_tools
{

/////////////////////////////////////////////////
std::time_t ParseDateTime(const std::string &_datetime)
{
  int y, M, d, h, m;
  float s;
  sscanf(_datetime.c_str(), "%d-%d-%dT%d:%d:%fZ", &y, &M, &d, &h, &m, &s);
  std::tm tm;
  // Year since 1900
  tm.tm_year = y - 1900;
  // 0-11
  tm.tm_mon = M - 1;
  // 1-31
  tm.tm_mday = d;
  // 0-23
  tm.tm_hour = h;
  // 0-59
  tm.tm_min = m;
  // 0-61 (0-60 in C++11)
  tm.tm_sec = static_cast<int>(s);
  // 0 - standard time, 1, daylight saving, -1 unknown
  tm.tm_isdst = -1;
  // cppcheck-suppress ConfigurationNotChecked
  return timegm(&tm);
}

/////////////////////////////////////////////////
/// \brief Parse a JSON object as a license.
/// \param[in] _json JSON object containing a single license.
/// \param[out] _license License information where the first value in
/// the pair is the name of the license and the second value is
/// the license id on the Fuel server.
/// \return True if parsing succeeded or false otherwise
bool ParseLicenseImpl(const Json::Value &_json,
    std::pair<std::string, unsigned int> &_license)
{
  try
  {
    if (!_json.isObject())
    {
      gzerr << "License isn't a json object!\n";
      return false;
    }

    if (_json.isMember("name"))
      _license.first = _json["name"].asString();
    if (_json.isMember("ID"))
      _license.second = _json["ID"].asUInt();
  }
#if GZ_JSON_HAVE_EXCEPTIONS == 1
  catch (...)
  {
    std::string what;
#else
  catch (const Json::LogicError &error)
  {
    std::string what = ": [" + std::string(error.what()) + "]";
#endif
    gzerr << "Bad response from server" << what << "\n";
    return false;
  }

  return true;
}

/////////////////////////////////////////////////
std::vector<std::string> JSONParser::ParseTags(const Json::Value &_json)
{
  std::vector<std::string> tags;
  if (!_json.isArray())
  {
    gzerr << "JSON tags are not an array\n";
    return tags;
  }

  try
  {
    for (auto tagIt = _json.begin(); tagIt != _json.end(); ++tagIt)
      tags.push_back(tagIt->asString());
  }
#if GZ_JSON_HAVE_EXCEPTIONS == 1
  catch (...)
  {
    std::string what;
#else
  catch (const Json::LogicError &error)
  {
    std::string what = ": [" + std::string(error.what()) + "]";
#endif
    gzerr << "Exception parsing tags" << what << "\n";
    return std::vector<std::string>();
  }

  return tags;
}

/////////////////////////////////////////////////
ModelIdentifier JSONParser::ParseModel(const std::string &_json,
  const ServerConfig &_server)
{
  Json::CharReaderBuilder reader;
  Json::Value model;
  ModelIdentifier id;
  std::istringstream iss(_json);
  JSONCPP_STRING errs;

  Json::parseFromStream(reader, iss, &model, &errs);
  ParseModelImpl(model, id);

  // Adding the server used to retrieve the model.
  id.SetServer(_server);

  return id;
}

/////////////////////////////////////////////////
std::vector<ModelIdentifier> JSONParser::ParseModels(const std::string &_json,
  const ServerConfig &_server)
{
  std::vector<ModelIdentifier> ids;
  Json::CharReaderBuilder reader;
  Json::Value models;
  std::istringstream iss(_json);
  JSONCPP_STRING errs;

  Json::parseFromStream(reader, iss, &models, &errs);

  if (!models.isArray())
  {
    gzerr << "JSON response is not an array\n";
  }
  else
  {
    for (auto modelIt = models.begin(); modelIt != models.end(); ++modelIt)
    {
      Json::Value model = *modelIt;
      ModelIdentifier id;
      if (!ParseModelImpl(model, id))
      {
        gzerr << "Model isn't a json object!\n";
        break;
      }

      // Adding the server used to retrieve the model.
      id.SetServer(_server);

      ids.push_back(id);
    }
  }

  return ids;
}

/////////////////////////////////////////////////
bool JSONParser::ParseModelImpl(
  const Json::Value &_json, ModelIdentifier &_model)
{
  try
  {
    if (!_json.isObject())
    {
      gzerr << "Model isn't a json object!\n";
      return false;
    }

    if (_json.isMember("name"))
      _model.SetName(_json["name"].asString());
    if (_json.isMember("owner"))
      _model.SetOwner(_json["owner"].asString());
    if (_json.isMember("updatedAt"))
      _model.SetModifyDate(ParseDateTime(_json["updatedAt"].asString()));
    if (_json.isMember("createdAt"))
      _model.SetUploadDate(ParseDateTime(_json["createdAt"].asString()));
    if (_json.isMember("description"))
      _model.SetDescription(_json["description"].asString());
    if (_json.isMember("likes"))
      _model.SetLikeCount(_json["likes"].asUInt());
    if (_json.isMember("downloads"))
      _model.SetDownloadCount(_json["downloads"].asUInt());
    if (_json.isMember("filesize"))
      _model.SetFileSize(_json["filesize"].asUInt());
    if (_json.isMember("license_name"))
      _model.SetLicenseName(_json["license_name"].asString());
    if (_json.isMember("license_url"))
      _model.SetLicenseUrl(_json["license_url"].asString());
    if (_json.isMember("license_image"))
      _model.SetLicenseImageUrl(_json["license_image"].asString());
    if (_json.isMember("tags"))
      _model.SetTags(ParseTags(_json["tags"]));
    if (_json.isMember("version"))
      _model.SetVersion(_json["version"].asUInt());
  }
#if GZ_JSON_HAVE_EXCEPTIONS == 1
  catch (...)
  {
    std::string what;
#else
  catch (const Json::LogicError &error)
  {
    std::string what = ": [" + std::string(error.what()) + "]";
#endif
    gzerr << "Bad response from server" << what << "\n";
    return false;
  }

  return true;
}

/////////////////////////////////////////////////
std::string JSONParser::BuildModel(ModelIter _modelIt)
{
  ModelIdentifier id = _modelIt->Identification();
  Json::Value value;
  value["name"] = id.Name();
  value["description"] = id.Description();
  value["version"] = id.Version();

  Json::StreamWriterBuilder builder;
  return Json::writeString(builder, value);
}

/////////////////////////////////////////////////
WorldIdentifier JSONParser::ParseWorld(const std::string &_json,
  const ServerConfig &_server)
{
  Json::CharReaderBuilder reader;
  Json::Value world;
  WorldIdentifier id;
  std::istringstream iss(_json);
  JSONCPP_STRING errs;

  Json::parseFromStream(reader, iss, &world, &errs);
  ParseWorldImpl(world, id);

  // Adding the server used to retrieve the world.
  id.SetServer(_server);

  return id;
}

/////////////////////////////////////////////////
std::vector<WorldIdentifier> JSONParser::ParseWorlds(const std::string &_json,
  const ServerConfig &_server)
{
  std::vector<WorldIdentifier> ids;
  Json::CharReaderBuilder reader;
  Json::Value worlds;
  std::istringstream iss(_json);
  JSONCPP_STRING errs;

  Json::parseFromStream(reader, iss, &worlds, &errs);

  if (!worlds.isArray())
  {
    gzerr << "JSON response is not an array\n";
  }
  else
  {
    for (auto worldIt = worlds.begin(); worldIt != worlds.end(); ++worldIt)
    {
      Json::Value world = *worldIt;
      WorldIdentifier id;
      if (!ParseWorldImpl(world, id))
      {
        gzerr << "World isn't a json object!\n";
        break;
      }

      // Adding the server used to retrieve the world.
      id.SetServer(_server);

      ids.push_back(id);
    }
  }

  return ids;
}

/////////////////////////////////////////////////
bool JSONParser::ParseWorldImpl(
  const Json::Value &_json, WorldIdentifier &_world)
{
  try
  {
    if (!_json.isObject())
    {
      gzerr << "World isn't a json object!\n";
      return false;
    }

    if (_json.isMember("name"))
      _world.SetName(_json["name"].asString());
    if (_json.isMember("owner"))
      _world.SetOwner(_json["owner"].asString());
    if (_json.isMember("version"))
      _world.SetVersion(_json["version"].asUInt());
  }
#if GZ_JSON_HAVE_EXCEPTIONS == 1
  catch (...)
  {
    std::string what;
#else
  catch (const Json::LogicError &error)
  {
    std::string what = ": [" + std::string(error.what()) + "]";
#endif
    gzerr << "Bad response from server" << what << "\n";
    return false;
  }

  return true;
}

/////////////////////////////////////////////////
std::string JSONParser::BuildWorld(WorldIter _worldIt)
{
  Json::Value value;
  value["name"] = _worldIt->Name();
  value["version"] = _worldIt->Version();

  Json::StreamWriterBuilder builder;
  return Json::writeString(builder, value);
}

/////////////////////////////////////////////////
bool JSONParser::ParseLicenses(const std::string &_json,
    std::map<std::string, unsigned int> &_licenses)
{
  Json::CharReaderBuilder reader;
  Json::Value licenses;
  std::istringstream iss(_json);
  JSONCPP_STRING errs;
  Json::parseFromStream(reader, iss, &licenses, &errs);

  if (!licenses.isArray())
  {
    gzerr << "JSON response is not an array.\n";
    return false;
  }

  for (auto licenseIt = licenses.begin();
      licenseIt != licenses.end(); ++licenseIt)
  {
    Json::Value licenseJson = *licenseIt;
    std::pair<std::string, unsigned int> license;
    if (!ParseLicenseImpl(licenseJson, license))
    {
      gzerr << "License isn't a json object!\n";
      continue;
    }

    _licenses.insert(license);
  }

  return true;
}
}  // namespace gz::fuel_tools
