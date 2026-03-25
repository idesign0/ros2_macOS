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

#include <gz/plugin/WeakPluginPtr.hh>
#include <gz/plugin/Loader.hh>

#include "../plugins/DummyPlugins.hh"
#include "utils.hh"

/////////////////////////////////////////////////
TEST(WeakPluginPtr, Lifecycle)
{
  const std::string &libraryPath = GzDummyPlugins_LIB;

  gz::plugin::WeakPluginPtr weak;

  CHECK_FOR_LIBRARY(libraryPath, false);

  {
    gz::plugin::Loader pl;
    pl.LoadLib(libraryPath);
    CHECK_FOR_LIBRARY(libraryPath, true);

    gz::plugin::PluginPtr plugin =
        pl.Instantiate("test::util::DummyMultiPlugin");

    weak = plugin;

    EXPECT_FALSE(weak.IsExpired());

    gz::plugin::PluginPtr copy = weak.Lock();

    test::util::DummyIntBase* base =
        copy->QueryInterface<test::util::DummyIntBase>();

    EXPECT_TRUE(base);
    EXPECT_EQ(5, base->MyIntegerValueIs());
    EXPECT_EQ(plugin->QueryInterface<test::util::DummyIntBase>(), base);

    CHECK_FOR_LIBRARY(libraryPath, true);
  }

  CHECK_FOR_LIBRARY(libraryPath, false);

  EXPECT_TRUE(weak.IsExpired());

  gz::plugin::PluginPtr empty = weak.Lock();
  EXPECT_TRUE(empty.IsEmpty());
  EXPECT_FALSE(empty->QueryInterface<test::util::DummyIntBase>());
}

/////////////////////////////////////////////////
TEST(WeakPluginPtr, CopyMove)
{
  gz::plugin::Loader pl;
  pl.LoadLib(GzDummyPlugins_LIB);

  gz::plugin::PluginPtr plugin =
      pl.Instantiate("test::util::DummyMultiPlugin");

  gz::plugin::WeakPluginPtr weakConstructFromPlugin(plugin);
  EXPECT_EQ(plugin, weakConstructFromPlugin.Lock());

  gz::plugin::WeakPluginPtr weakCopyFromOther(
        weakConstructFromPlugin);
  EXPECT_EQ(plugin, weakConstructFromPlugin.Lock());

  gz::plugin::WeakPluginPtr weakCopyAssignFromOther;
  weakCopyAssignFromOther = weakConstructFromPlugin;
  EXPECT_EQ(plugin, weakCopyAssignFromOther.Lock());

  gz::plugin::WeakPluginPtr weakMoveFromOther(
        std::move(weakCopyFromOther));
  EXPECT_EQ(plugin, weakMoveFromOther.Lock());

  gz::plugin::WeakPluginPtr weakMoveAssignFromOther;
  weakMoveAssignFromOther = std::move(weakCopyAssignFromOther);
  EXPECT_EQ(plugin, weakMoveAssignFromOther.Lock());

  gz::plugin::WeakPluginPtr weakAssignFromPlugin;
  weakAssignFromPlugin = plugin;
  EXPECT_EQ(plugin, weakAssignFromPlugin.Lock());
}
