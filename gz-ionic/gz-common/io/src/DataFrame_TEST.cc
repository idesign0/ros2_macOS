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
#include <iostream>
#include <sstream>
#include <string>

#include <gz/math/TimeVaryingVolumetricGrid.hh>

#include "gz/common/CSVStreams.hh"
#include "gz/common/DataFrame.hh"
#include "gz/common/Filesystem.hh"

using namespace gz;

/////////////////////////////////////////////////
TEST(DataFrameTests, SimpleCSV)
{
  std::stringstream ss;
  ss << "t,x,y,z,temperature" << std::endl
     << "0,0,0,0,25.2" << std::endl
     << "0,10,0,0,25.2" << std::endl
     << "0,0,10,0,25.2" << std::endl
     << "0,10,10,0,25.2" << std::endl
     << "1,0,0,0,24.9" << std::endl
     << "1,10,0,0,24.9" << std::endl
     << "1,0,10,0,25.1" << std::endl
     << "1,10,10,0,25.1" << std::endl;

  using DataT =
      math::InMemoryTimeVaryingVolumetricGrid<double, double, double>;
  using DataFrameT = common::DataFrame<std::string, DataT>;
  const auto df = common::IO<DataFrameT>::ReadFrom(
      common::CSVIStreamIterator(ss),
      common::CSVIStreamIterator());

  ASSERT_TRUE(df.Has("temperature"));
  const DataT &temperatureData = df["temperature"];
  auto temperatureSession = temperatureData.StepTo(
      temperatureData.CreateSession(), 0.);
  ASSERT_TRUE(temperatureSession.has_value());
  const math::Vector3d position{5., 5., 0.};
  auto temperature = temperatureData.LookUp(
      temperatureSession.value(), position);
  ASSERT_TRUE(temperature.has_value());
  EXPECT_DOUBLE_EQ(25.2, temperature.value());
  temperatureSession = temperatureData.StepTo(
      temperatureData.CreateSession(), 0.5);
  ASSERT_TRUE(temperatureSession.has_value());
  temperature = temperatureData.LookUp(
      temperatureSession.value(), position);
  ASSERT_TRUE(temperature.has_value());
  EXPECT_DOUBLE_EQ(25.1, temperature.value());
  auto keys = df.Keys();
  ASSERT_EQ(keys.size(), 1);
  ASSERT_EQ(keys[0], "temperature");
}

/////////////////////////////////////////////////
TEST(DataFrameTests, ComplexCSV)
{
  std::stringstream ss;
  ss << "timestamp,temperature,pressure,humidity,lat,lon,altitude" << std::endl
     << "1658923062,13.1,101490,91,36.80029505,-121.788972517,0.8" << std::endl
     << "1658923062,13,101485,88,36.80129505,-121.788972517,0.8" << std::endl
     << "1658923062,13.1,101485,89,36.80029505,-121.789972517,0.8" << std::endl
     << "1658923062,13.5,101490,92,36.80129505,-121.789972517,0.8" << std::endl;

  using DataT =
      math::InMemoryTimeVaryingVolumetricGrid<double, double, double>;
  using DataFrameT = common::DataFrame<std::string, DataT>;
  const auto df = common::IO<DataFrameT>::ReadFrom(
      common::CSVIStreamIterator(ss), common::CSVIStreamIterator(),
      "timestamp", {"lat", "lon", "altitude"});
  EXPECT_TRUE(df.Has("temperature"));
  EXPECT_TRUE(df.Has("humidity"));
  ASSERT_TRUE(df.Has("pressure"));
  auto keys = df.Keys();
  ASSERT_EQ(keys.size(), 3);
  const DataT &pressureData = df["pressure"];
  auto pressureSession = pressureData.CreateSession();
  const math::Vector3d position{36.80079505, -121.789472517, 0.8};
  auto pressure = pressureData.LookUp(pressureSession, position);
  ASSERT_TRUE(pressure.has_value());
  EXPECT_DOUBLE_EQ(101487.5, pressure.value());
}
