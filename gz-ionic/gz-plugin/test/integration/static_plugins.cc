/*
 * Copyright (C) 2022 Open Source Robotics Foundation
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

#include <gz/plugin/Loader.hh>

#include "../plugins/DummyPlugins.hh"

/////////////////////////////////////////////////
TEST(StaticPlugins, Load)
{
  gz::plugin::Loader loader;
  EXPECT_EQ(loader.AllPlugins().size(), 1);
  EXPECT_EQ(loader.LookupPlugin("Alternative name"),
      "test::util::DummySinglePlugin");
}

TEST(StaticPlugins, AdditionalAliases)
{
  gz::plugin::Loader loader;
  EXPECT_EQ(loader.LookupPlugin("Baz"), "test::util::DummySinglePlugin");
  EXPECT_EQ(loader.LookupPlugin("Bar"), "test::util::DummySinglePlugin");
}

TEST(StaticPlugins, Interfaces)
{
  gz::plugin::Loader loader;
  EXPECT_EQ(loader.PluginsImplementing<test::util::DummyNameBase>().size(),
      1);
}

TEST(StaticPlugins, Instantiate)
{
  gz::plugin::Loader loader;
  std::string resolvedName = loader.LookupPlugin("Alternative name");
  auto pluginInstance = loader.Instantiate(resolvedName);
  EXPECT_TRUE(pluginInstance);
  auto *dummySinglePluginInterface =
      pluginInstance->QueryInterface<test::util::DummyNameBase>();
  EXPECT_EQ(dummySinglePluginInterface->MyNameIs(), "DummySinglePlugin");
}
