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

#include <gtest/gtest.h>

#include <gz/plugin/Loader.hh>

#include "../plugins/DummyPlugins.hh"

/////////////////////////////////////////////////
TEST(Alias, InspectAliases)
{
  gz::plugin::Loader pl;

  // Make sure the expected plugins were loaded.
  std::unordered_set<std::string> pluginNames =
      pl.LoadLib(GzDummyPlugins_LIB);
  ASSERT_EQ(1u, pluginNames.count("test::util::DummySinglePlugin"));
  ASSERT_EQ(1u, pluginNames.count("test::util::DummyMultiPlugin"));
  ASSERT_EQ(1u, pluginNames.count("test::util::DummyNoAliasPlugin"));

  EXPECT_EQ(0u, pl.PluginsWithAlias("fake alias").size());
  EXPECT_EQ(1u, pl.PluginsWithAlias("Alternative name").size());
  EXPECT_EQ(2u, pl.PluginsWithAlias("Bar").size());
  EXPECT_EQ(2u, pl.PluginsWithAlias("Baz").size());
  EXPECT_EQ(1u, pl.PluginsWithAlias("Foo").size());
  EXPECT_EQ(1u, pl.PluginsWithAlias("test::util::DummySinglePlugin").size());

  EXPECT_EQ(3u, pl.AliasesOfPlugin("test::util::DummySinglePlugin").size());
  EXPECT_EQ(3u, pl.AliasesOfPlugin("test::util::DummyMultiPlugin").size());
  EXPECT_EQ(0u, pl.AliasesOfPlugin("test::util::DummyNoAliasPlugin").size());
  EXPECT_EQ(0u, pl.AliasesOfPlugin("fake::plugin").size());
}

/////////////////////////////////////////////////
TEST(Alias, ConflictingAlias)
{
  gz::plugin::Loader pl;

  // Make sure the expected plugins were loaded.
  std::unordered_set<std::string> pluginNames =
      pl.LoadLib(GzDummyPlugins_LIB);
  ASSERT_EQ(1u, pluginNames.count("test::util::DummySinglePlugin"));
  ASSERT_EQ(1u, pluginNames.count("test::util::DummyMultiPlugin"));
  ASSERT_EQ(1u, pluginNames.count("test::util::DummyNoAliasPlugin"));

  // This alias conflicts between DummySinglePlugin and DummyMultiPlugin
  gz::plugin::PluginPtr attempt = pl.Instantiate("Bar");
  EXPECT_TRUE(attempt.IsEmpty());

  // This alias conflicts between DummySinglePlugin and DummyMultiPlugin
  attempt = pl.Instantiate("Baz");
  EXPECT_TRUE(attempt.IsEmpty());

  // This alias refers only to DummySinglePlugin
  attempt = pl.Instantiate("Alternative name");
  EXPECT_FALSE(attempt.IsEmpty());

  auto * interface = attempt->QueryInterface<test::util::DummyNameBase>();
  ASSERT_NE(nullptr, interface);
  EXPECT_EQ("DummySinglePlugin", interface->MyNameIs());

  // This alias refers only to DummyMultiPlugin
  attempt = pl.Instantiate("Foo");
  EXPECT_FALSE(attempt.IsEmpty());

  interface = attempt->QueryInterface<test::util::DummyNameBase>();
  ASSERT_NE(nullptr, interface);
  EXPECT_EQ("DummyMultiPlugin", interface->MyNameIs());

  attempt = pl.Instantiate("not a plugin");
  EXPECT_TRUE(attempt.IsEmpty());
}
