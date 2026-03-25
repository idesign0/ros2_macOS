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

#include <gz/plugin/EnablePluginFromThis.hh>
#include <gz/plugin/Loader.hh>
#include <gz/plugin/SpecializedPluginPtr.hh>
#include <gz/plugin/WeakPluginPtr.hh>

#include "../plugins/DummyPlugins.hh"
#include "utils.hh"

/////////////////////////////////////////////////
TEST(EnablePluginFromThis, BasicInstantiate)
{
  gz::plugin::Loader pl;
  pl.LoadLib(GzDummyPlugins_LIB);

  gz::plugin::PluginPtr plugin =
      pl.Instantiate("test::util::DummyMultiPlugin");
  ASSERT_TRUE(plugin);

  auto *fromThisInterface =
      plugin->QueryInterface<gz::plugin::EnablePluginFromThis>();
  EXPECT_TRUE(fromThisInterface);

  gz::plugin::PluginPtr fromThis = fromThisInterface->PluginFromThis();
  EXPECT_EQ(plugin, fromThis);

  gz::plugin::ConstPluginPtr constFromThis =
      static_cast<const gz::plugin::EnablePluginFromThis*>(
        fromThisInterface)->PluginFromThis();
  EXPECT_EQ(constFromThis, fromThis);

  test::util::DummyGetPluginInstancePtr *getInstance =
      plugin->QueryInterface<test::util::DummyGetPluginInstancePtr>();
  ASSERT_TRUE(getInstance);
  std::shared_ptr<void> ptr = getInstance->PluginInstancePtr();
  EXPECT_NE(nullptr, ptr);


  // Note: the DummySinglePlugin class does not inherit EnablePluginFromThis
  plugin = pl.Instantiate("test::util::DummySinglePlugin");
  ASSERT_TRUE(plugin);

  fromThisInterface =
      plugin->QueryInterface<gz::plugin::EnablePluginFromThis>();
  EXPECT_EQ(nullptr, fromThisInterface);
}

/////////////////////////////////////////////////
using MySpecializedPluginPtr = gz::plugin::SpecializedPluginPtr<
  test::util::DummyNameBase,
  test::util::DummyDoubleBase,
  test::util::DummyIntBase
>;

/////////////////////////////////////////////////
TEST(EnablePluginFromThis, TemplatedInstantiate)
{
  gz::plugin::Loader pl;
  pl.LoadLib(GzDummyPlugins_LIB);

  MySpecializedPluginPtr plugin =
      pl.Instantiate<MySpecializedPluginPtr>("test::util::DummyMultiPlugin");
  ASSERT_TRUE(plugin);

  auto *fromThisInterface =
      plugin->QueryInterface<gz::plugin::EnablePluginFromThis>();
  EXPECT_TRUE(fromThisInterface);

  gz::plugin::PluginPtr fromThis = fromThisInterface->PluginFromThis();
  EXPECT_EQ(plugin, fromThis);


  // Note: the DummySinglePlugin class does not inherit EnablePluginFromThis
  plugin = pl.Instantiate<MySpecializedPluginPtr>(
        "test::util::DummySinglePlugin");
  ASSERT_TRUE(plugin);

  fromThisInterface =
      plugin->QueryInterface<gz::plugin::EnablePluginFromThis>();
  EXPECT_EQ(nullptr, fromThisInterface);
}

/////////////////////////////////////////////////
TEST(EnablePluginFromThis, LibraryManagement)
{
  const std::string &libraryPath = GzDummyPlugins_LIB;

  gz::plugin::WeakPluginPtr weak;

  {
    gz::plugin::PluginPtr longterm;

    CHECK_FOR_LIBRARY(libraryPath, false);

    {
      gz::plugin::Loader pl;
      pl.LoadLib(libraryPath);

      CHECK_FOR_LIBRARY(libraryPath, true);

      gz::plugin::PluginPtr temporary =
          pl.Instantiate("test::util::DummyMultiPlugin");

      auto fromThis =
          temporary->QueryInterface<gz::plugin::EnablePluginFromThis>();

      longterm = fromThis->PluginFromThis();
      weak = fromThis->PluginFromThis();

      EXPECT_FALSE(weak.IsExpired());
      EXPECT_FALSE(longterm.IsEmpty());
    }

    EXPECT_FALSE(weak.IsExpired());
    EXPECT_FALSE(longterm.IsEmpty());

    CHECK_FOR_LIBRARY(libraryPath, true);
  }

  EXPECT_TRUE(weak.IsExpired());

  CHECK_FOR_LIBRARY(libraryPath, false);
}
