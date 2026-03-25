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
#include <cstdio>
#include <string>

#include "gz/fuel_tools/ClientConfig.hh"
#include "gz/fuel_tools/ModelIter.hh"

#include "ModelIterPrivate.hh"
#include "ModelPrivate.hh"

using namespace gz;
using namespace fuel_tools;

namespace gz
{
  namespace fuel_tools
  {
    /// \brief Forward Declaration
    class ModelIterTestFixture;

    /// \brief Class to misuse friendship
    class ModelIterTest
    {
      public: static ModelIter EmptyModelIter()
        {
          return ModelIterFactory::Create();
        }

      public: static ModelIter ModelIterThreeModelIds()
        {
          gz::fuel_tools::ServerConfig srv;
          srv.SetUrl(common::URI("https://gazebosim.org"));

          std::vector<ModelIdentifier> ids;
          for (int i = 0; i < 3; i++)
          {
            ModelIdentifier id;
            char buf[10];
            std::snprintf(buf, sizeof(buf), "model%d", i);
            std::string name = buf;
            std::snprintf(buf, sizeof(buf), "owner%d", i);
            std::string owner = buf;
            id.SetName(name);
            id.SetOwner(owner);
            id.SetServer(srv);
            ids.push_back(id);
          }
          return ModelIterFactory::Create(ids);
        }

      public: static ModelIter ModelIterThreeModels()
        {
          gz::fuel_tools::ServerConfig srv;
          srv.SetUrl(common::URI("https://gazebosim.org"));

          std::vector<Model> models;
          for (int i = 0; i < 3; i++)
          {
            ModelIdentifier id;
            char buf[10];
            std::snprintf(buf, sizeof(buf), "model%d", i);
            std::string name = buf;
            std::snprintf(buf, sizeof(buf), "owner%d", i);
            std::string owner = buf;

            std::shared_ptr<ModelPrivate> ptr(new ModelPrivate);
            ptr->id.SetName(name);
            ptr->id.SetOwner(owner);
            ptr->id.SetServer(srv);

            models.push_back(Model(ptr));
          }
          return ModelIterFactory::Create(models);
        }
    };
  }
}

/////////////////////////////////////////////////
/// \brief No ids, iter should evaluate to false
TEST(ModelIterTestFixture, FalseIfNoModels)
{
  EXPECT_FALSE(ModelIterTest::EmptyModelIter());
}

/////////////////////////////////////////////////
TEST(ModelIterTestFixture, ConstBool)
{
  const ModelIter constIter = ModelIterTest::EmptyModelIter();
  EXPECT_FALSE(constIter);
}

/////////////////////////////////////////////////
/// \brief Iter should be ready to move if there are ids
TEST(ModelIterTestFixture, TrueIfSomeModels)
{
  EXPECT_TRUE(ModelIterTest::ModelIterThreeModelIds());
}

/////////////////////////////////////////////////
/// \brief Iter should move through 3 ids
TEST(ModelIterTestFixture, MoveThroughIds)
{
  ModelIter iter = ModelIterTest::ModelIterThreeModelIds();
  EXPECT_TRUE(iter);
  EXPECT_EQ("model0", iter->Identification().Name());
  EXPECT_EQ("model0", (*iter).Identification().Name());

  ++iter;
  EXPECT_TRUE(iter);
  EXPECT_EQ("model1", iter->Identification().Name());
  EXPECT_EQ("model1", (*iter).Identification().Name());

  ++iter;
  EXPECT_TRUE(iter);
  EXPECT_EQ("model2", iter->Identification().Name());
  EXPECT_EQ("model2", (*iter).Identification().Name());

  ++iter;
  EXPECT_FALSE(iter);
}

/////////////////////////////////////////////////
/// \brief Iter should move through 3 models
TEST(ModelIterTestFixture, MoveThroughModels)
{
  ModelIter iter = ModelIterTest::ModelIterThreeModels();
  EXPECT_TRUE(iter);
  EXPECT_EQ("model0", iter->Identification().Name());
  EXPECT_EQ("model0", (*iter).Identification().Name());

  ++iter;
  EXPECT_TRUE(iter);
  EXPECT_EQ("model1", iter->Identification().Name());
  EXPECT_EQ("model1", (*iter).Identification().Name());

  ++iter;
  EXPECT_TRUE(iter);
  EXPECT_EQ("model2", iter->Identification().Name());
  EXPECT_EQ("model2", (*iter).Identification().Name());

  ++iter;
  EXPECT_FALSE(iter);
}
