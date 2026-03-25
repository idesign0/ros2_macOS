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

#ifndef GZ_PLUGIN_TEST_INTEGRATION_PLUGIN_UNLOAD_HH
#define GZ_PLUGIN_TEST_INTEGRATION_PLUGIN_UNLOAD_HH

#include <gtest/gtest.h>

#include <iostream>
#include <string>
#include <unordered_set>

#include "../plugins/InstanceCounter.hh"
#include "gz/plugin/Loader.hh"
#include "gz/plugin/PluginPtr.hh"
#include "gz/plugin/SpecializedPluginPtr.hh"

/////////////////////////////////////////////////
/// \brief Load the InstanceCounter plugin
/// \param[in] _nodelete True if RTLD_NODELETE should be used when loading the
/// \return Pointer to the plugin
inline gz::plugin::PluginPtr LoadInstanceCounter(bool _nodelete)
{
  gz::plugin::Loader pl;

  std::unordered_set<std::string> pluginNames =
      pl.LoadLib(GzInstanceCounter_LIB, _nodelete);

  return pl.Instantiate("test::util::InstanceCounter");
}

/////////////////////////////////////////////////
/// \brief Load plugin, Instantiate the InstanceCounter, check the number of
/// instances, and finally unload the plugin. Note, the plugin is unloaded when
/// `instanceCounterPlugin` goes out of scope.
/// \param[in] _nodelete True if RTLD_NODELETE should be used when loading the
/// library.
/// \param[in] _numExpectedInstances Expected number of instances of the plugin.
inline void LoadAndTestInstanceCounter(bool _nodelete,
                                       int _numExpectedInstances)
{
  gz::plugin::PluginPtr instanceCounterPlugin = LoadInstanceCounter(_nodelete);
  test::util::InstanceCounterBase *instanceCounter =
      instanceCounterPlugin->QueryInterface<test::util::InstanceCounterBase>();
  ASSERT_NE(nullptr, instanceCounter);

  EXPECT_EQ(_numExpectedInstances, instanceCounter->Instances());
}

#endif
