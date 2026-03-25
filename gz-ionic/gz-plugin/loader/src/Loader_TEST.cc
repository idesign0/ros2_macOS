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

#include <algorithm>

#include <gz/plugin/Loader.hh>
#include <gz/plugin/config.hh>

#include <gz/plugin/SpecializedPluginPtr.hh>

/////////////////////////////////////////////////
TEST(Loader, InitialNoInterfacesImplemented)
{
  gz::plugin::Loader loader;
  EXPECT_EQ(0u, loader.InterfacesImplemented().size());
}

/////////////////////////////////////////////////
TEST(Loader, LoadNonexistantLibrary)
{
  gz::plugin::Loader loader;
  EXPECT_TRUE(loader.LoadLib("/path/to/libDoesNotExist.so").empty());
  EXPECT_FALSE(loader.ForgetLibrary("/path/to/libDoesNotExist.so"));
}

/////////////////////////////////////////////////
TEST(Loader, LoadNonLibrary)
{
  gz::plugin::Loader loader;
  EXPECT_TRUE(loader.LoadLib(std::string("")
                             + "/core/src/Plugin.cc").empty());
}

/////////////////////////////////////////////////
TEST(Loader, LoadNonPluginLibrary)
{
  gz::plugin::Loader loader;
  EXPECT_TRUE(loader.LoadLib("").empty());
}

/////////////////////////////////////////////////
TEST(Loader, InstantiateUnloadedPlugin)
{
  gz::plugin::Loader loader;
  gz::plugin::PluginPtr plugin =
      loader.Instantiate("plugin::that::is::not::loaded");
  EXPECT_FALSE(plugin);
  EXPECT_FALSE(loader.ForgetLibraryOfPlugin("plugin::that::is::not::loaded"));
}

class SomeInterface { };

/////////////////////////////////////////////////
TEST(Loader, InstantiateSpecializedPlugin)
{
  gz::plugin::Loader loader;

  // This makes sure that Loader::Instantiate can compile
  auto plugin = loader.Instantiate<
      gz::plugin::SpecializedPluginPtr<SomeInterface> >("fake::plugin");
  EXPECT_FALSE(plugin);
}

/////////////////////////////////////////////////
TEST(Loader, DoubleLoad)
{
  // We do this test for code coverage, to test the lines of code that get run
  // when a user asks for a library to be loaded twice.
  gz::plugin::Loader loader;

  loader.LoadLib(GzDummyPlugins_LIB);
  const std::size_t interfaceCount = loader.InterfacesImplemented().size();
  EXPECT_LT(0u, interfaceCount);

  loader.LoadLib(GzDummyPlugins_LIB);
  EXPECT_EQ(interfaceCount, loader.InterfacesImplemented().size());
}

/////////////////////////////////////////////////
TEST(Loader, ForgetUnloadedLibrary)
{
  // These tests are for line coverage.

  // This first test triggers lines for the case that:
  //   1. A library is not loaded, and
  //   2. We tell a loader to forget the library
  gz::plugin::Loader loader;
  EXPECT_FALSE(loader.ForgetLibrary(GzDummyPlugins_LIB));

  // This next test triggers lines for the case that:
  //   1. A library is loaded by some loader in the application, and
  //   2. We tell a *different* loader to forget the library
  gz::plugin::Loader hasTheLibrary;
  EXPECT_LT(0u, hasTheLibrary.LoadLib(GzDummyPlugins_LIB).size());

  EXPECT_FALSE(loader.ForgetLibrary(GzDummyPlugins_LIB));
}
