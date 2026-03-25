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

#include <gtest/gtest.h>

#include <ctime>
#include <sstream>
#include <string>
#include <vector>

#include "gz/fuel_tools/ClientConfig.hh"
#include "gz/fuel_tools/JSONParser.hh"
#include "gz/fuel_tools/ModelIter.hh"
#include "gz/fuel_tools/WorldIter.hh"
#include "ModelIterPrivate.hh"
#include "WorldIterPrivate.hh"

using namespace gz;
using namespace fuel_tools;

/////////////////////////////////////////////////
/// \brief Convert JSON string to model iterator
TEST(JSONParser, ParseModels)
{
  std::stringstream tmpJsonStr;
  tmpJsonStr << "["
    << "{\"id\":1,\"createdAt\":\"2012-04-21T19:25:44.511Z\","
    << "\"updatedAt\":\"2012-04-23T18:25:43.511Z\","
    << "\"name\":\"car\","
    << "\"version\":3}]";

  ServerConfig srv;
  srv.SetUrl(common::URI("banana://testServer"));

  auto modelIds = JSONParser::ParseModels(tmpJsonStr.str(), srv);
  EXPECT_EQ(1u, modelIds.size());
  auto model = modelIds.front();
  EXPECT_EQ("car", model.Name());
  EXPECT_EQ(3u, model.Version());
  EXPECT_EQ("banana://testServer", model.Server().Url().Str());
  auto t = model.ModifyDate();
  char buffer[100];
  std::strftime(buffer, sizeof(buffer), "%F %T", gmtime(&t));
  EXPECT_EQ("2012-04-23 18:25:43", std::string(buffer));
  t = model.UploadDate();
  std::strftime(buffer, sizeof(buffer), "%F %T", gmtime(&t));
  EXPECT_EQ("2012-04-21 19:25:44", std::string(buffer));
}

/////////////////////////////////////////////////
/// \brief Convert model iterator to JSON string
TEST(JSONParser, BuildModel)
{
  std::vector<ModelIdentifier> ids;
  ModelIdentifier id;
  id.SetName("house");
  id.SetVersion(5);
  id.SetDescription("affordable");
  ids.push_back(id);

  std::string jsonStr = JSONParser::BuildModel(ModelIterFactory::Create(ids));

  std::stringstream tmpJsonStr;
  tmpJsonStr
    << "{\n"
    << "\t\"description\" : \"affordable\",\n"
    << "\t\"name\" : \"house\",\n"
    << "\t\"version\" : 5\n"
    << "}";
  EXPECT_EQ(tmpJsonStr.str(), jsonStr);
}

/////////////////////////////////////////////////
TEST(JSONParser, ParseModel)
{
  std::stringstream tmpJsonStr;
  tmpJsonStr << "{\"id\":1,\"createdAt\":\"2012-04-21T19:25:44.511Z\","
    << "\"updatedAt\":\"2012-04-23T18:25:43.511Z\","
    << "\"name\":\"car\","
    << "\"version\":1,"
    << "\"tags\":[\"tag1\"]}";

  ServerConfig srv;
  srv.SetUrl(common::URI("http://testServer"));

  ModelIdentifier model = JSONParser::ParseModel(tmpJsonStr.str(), srv);
  EXPECT_EQ("car", model.Name());
  EXPECT_EQ(1u, model.Version());
  EXPECT_EQ("http://testServer", model.Server().Url().Str());
  auto t = model.ModifyDate();
  char buffer[100];
  std::strftime(buffer, sizeof(buffer), "%F %T", gmtime(&t));
  EXPECT_EQ("2012-04-23 18:25:43", std::string(buffer));
  t = model.UploadDate();
  std::strftime(buffer, sizeof(buffer), "%F %T", gmtime(&t));
  EXPECT_EQ("2012-04-21 19:25:44", std::string(buffer));

  ASSERT_FALSE(model.Tags().empty());
  EXPECT_EQ("tag1", model.Tags()[0]);
}

/////////////////////////////////////////////////
/// \brief Convert JSON string to world iterator
TEST(JSONParser, ParseWorlds)
{
  std::stringstream tmpJsonStr;
  tmpJsonStr << "["
    << "{\"name\":\"car\","
    << "\"version\":3}]";

  ServerConfig srv;
  srv.SetUrl(common::URI("banana://testServer"));

  auto worldIds = JSONParser::ParseWorlds(tmpJsonStr.str(), srv);
  EXPECT_EQ(1u, worldIds.size());
  auto world = worldIds.front();
  EXPECT_EQ("car", world.Name());
  EXPECT_EQ(3u, world.Version());
  EXPECT_EQ("banana://testServer", world.Server().Url().Str());
}

/////////////////////////////////////////////////
/// \brief Convert world iterator to JSON string
TEST(JSONParser, BuildWorld)
{
  std::vector<WorldIdentifier> ids;
  WorldIdentifier id;
  id.SetName("house");
  id.SetVersion(5);
  ids.push_back(id);

  std::string jsonStr = JSONParser::BuildWorld(WorldIterFactory::Create(ids));

  std::stringstream tmpJsonStr;
  tmpJsonStr
    << "{\n"
    << "\t\"name\" : \"house\",\n"
    << "\t\"version\" : 5\n"
    << "}";
  EXPECT_EQ(tmpJsonStr.str(), jsonStr);
}

/////////////////////////////////////////////////
TEST(JSONParser, ParseWorld)
{
  std::stringstream tmpJsonStr;
  tmpJsonStr << "{\"name\":\"car\","
    << "\"version\":1]}";

  ServerConfig srv;
  srv.SetUrl(common::URI("banana://testServer"));

  WorldIdentifier world = JSONParser::ParseWorld(tmpJsonStr.str(), srv);
  EXPECT_EQ("car", world.Name());
  EXPECT_EQ(1u, world.Version());
  EXPECT_EQ("banana://testServer", world.Server().Url().Str());
}

/////////////////////////////////////////////////
TEST(JSONParser, ParseLicenses)
{
  std::stringstream tmpJsonStr;

  // This is the exact return value from
  // https://fuel.gazebosim.org/1.0/licenses as of April 29th, 2020.
  tmpJsonStr << R"tmpJsonStr([{
    "ID": 1,
    "CreatedAt": "2017-12-19T20:32:48Z",
    "UpdatedAt": "2020-04-29T12:01:25Z",
    "DeletedAt": null,
    "name": "Creative Commons - Public Domain",
    "url": "https://creativecommons.org/publicdomain/zero/1.0/",
    "image_url": "https://i.creativecommons.org/p/88x31.png"
    },
    {
    "ID": 2,
    "CreatedAt": "2017-12-19T20:32:48Z",
    "UpdatedAt": "2020-04-29T12:00:19Z",
    "DeletedAt": null,
    "name": "Creative Commons - Attribution",
    "url": "http://creativecommons.org/licenses/by/4.0/",
    "image_url": "https://i.creativecommons.org/l/by/4.0/88x31.png"
    },
    {
    "ID": 3,
    "CreatedAt": "2017-12-19T20:32:48Z",
    "UpdatedAt": "2017-12-19T20:32:48Z",
    "DeletedAt": null,
    "name": "Creative Commons - Attribution - Share Alike",
    "url": "http://creativecommons.org/licenses/by-sa/4.0/",
    "image_url": "https://i.creativecommons.org/l/by-sa/4.0/88x31.png"
    },
    {
    "ID": 4,
    "CreatedAt": "2017-12-19T20:32:48Z",
    "UpdatedAt": "2017-12-19T20:32:48Z",
    "DeletedAt": null,
    "name": "Creative Commons - Attribution - No Derivatives",
    "url": "http://creativecommons.org/licenses/by-nd/4.0/",
    "image_url": "https://i.creativecommons.org/l/by-nd/4.0/88x31.png"
    },
    {
    "ID": 5,
    "CreatedAt": "2017-12-19T20:32:48Z",
    "UpdatedAt": "2017-12-19T20:32:48Z",
    "DeletedAt": null,
    "name": "Creative Commons - Attribution - Non Commercial",
    "url": "http://creativecommons.org/licenses/by-nc/4.0/",
    "image_url": "https://i.creativecommons.org/l/by-nc/4.0/88x31.png"
    },
    {
    "ID": 6,
    "CreatedAt": "2017-12-19T20:32:48Z",
    "UpdatedAt": "2017-12-19T20:32:48Z",
    "DeletedAt": null,
    "name": "Creative Commons - Attribution - Non Commercial - Share Alike",
    "url": "http://creativecommons.org/licenses/by-nc-sa/4.0/",
    "image_url": "https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png"
    },
    {
    "ID": 7,
    "CreatedAt": "2017-12-19T20:32:48Z",
    "UpdatedAt": "2019-11-09T05:01:35Z",
    "DeletedAt": null,
    "name": "Creative Commons - Attribution - Non Commercial - No Derivatives",
    "url": "http://creativecommons.org/licenses/by-nc-nd/4.0/",
    "image_url": "https://i.creativecommons.org/l/by-nc-nd/4.0/88x31.png"
    }
    ])tmpJsonStr";


  std::map<std::string, unsigned int> licenses;
  EXPECT_TRUE(JSONParser::ParseLicenses(tmpJsonStr.str(), licenses));

  EXPECT_EQ(7u, licenses.size());
  for (unsigned int i = 1; i < 8; ++i)
  {
    switch (i)
    {
      case 1:
        EXPECT_EQ(i, licenses["Creative Commons - Public Domain"]);
        break;
      case 2:
        EXPECT_EQ(i, licenses["Creative Commons - Attribution"]);
        break;
      case 3:
        EXPECT_EQ(i, licenses["Creative Commons - Attribution - Share Alike"]);
        break;
      case 4:
        EXPECT_EQ(i,
            licenses["Creative Commons - Attribution - No Derivatives"]);
        break;
      case 5:
        EXPECT_EQ(i,
            licenses["Creative Commons - Attribution - Non Commercial"]);
        break;
      case 6:
        EXPECT_EQ(i, licenses[
            "Creative Commons - Attribution - Non Commercial - Share Alike"]);
        break;
      case 7:
        EXPECT_EQ(i, licenses[
           "Creative Commons - Attribution - Non Commercial - No Derivatives"]);
        break;
      default:
        FAIL() << "Invalid license id[" << i << "]\n";
        break;
    }
  }
}
