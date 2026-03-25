/*
 * Copyright (C) 2016 Open Source Robotics Foundation
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
#include <cstddef>
#include <filesystem>

#include "gz/msgs/MessageTypes.hh"
#include "gz/msgs/vector3d.pb.h"
#include "gz/msgs/serialized_map.pb.h"
#include "gz/msgs/Factory.hh"

namespace {
constexpr const char * kMsgsTestPath = GZ_MSGS_TEST_PATH;

#ifdef _WIN32
constexpr char kEnvironmentVariableSeparator = ';';
#else
constexpr char kEnvironmentVariableSeparator = ':';
#endif
}  // namespace


using Factory = gz::msgs::Factory;
using Vector3d = gz::msgs::Vector3d;

/////////////////////////////////////////////////
TEST(FactoryTest, Type)
{
  std::vector<std::string> types;
  Factory::Types(types);
  EXPECT_FALSE(types.empty());
  EXPECT_TRUE(std::find(types.begin(), types.end(),
        std::string("gz.msgs.Vector3d")) !=
      types.end());
}

/////////////////////////////////////////////////
TEST(FactoryTest, New)
{
  // Preferred call, using periods, fully qualified
  auto msg = Factory::New<Vector3d>("gz.msgs.Vector3d");
  ASSERT_TRUE(msg.get() != nullptr);

  // Various other supported ways of specifying gz.msgs
  msg = Factory::New<Vector3d>("gz_msgs.Vector3d");
  EXPECT_TRUE(msg.get() != nullptr);

  msg = Factory::New<Vector3d>(".gz.msgs.Vector3d");
  EXPECT_TRUE(msg.get() != nullptr);

  msg = Factory::New<Vector3d>(".gz_msgs.Vector3d");
  EXPECT_TRUE(msg.get() != nullptr);
}

/////////////////////////////////////////////////
TEST(FactoryTest, NewWithWellFormedData)
{
  gz::msgs::Vector3d msg;
  msg.set_x(1.0);
  msg.set_y(2.0);
  msg.set_z(3.0);

  auto msgFilled = Factory::New<gz::msgs::Vector3d>(
      "gz.msgs.Vector3d", "x: 1.0, y: 2.0, z: 3.0");
  ASSERT_TRUE(nullptr != msgFilled);

  EXPECT_DOUBLE_EQ(msg.x(), msgFilled->x());
  EXPECT_DOUBLE_EQ(msg.y(), msgFilled->y());
  EXPECT_DOUBLE_EQ(msg.z(), msgFilled->z());
}


/////////////////////////////////////////////////
TEST(FactoryTest, NewWithMalformedData)
{
  /// Passing bad data to New results in a nullptr
  auto msgFilled = Factory::New<gz::msgs::Vector3d>(
      "gz.msgs.Vector3d", "x: 1.0, y: asdf, z: 3.0");
  ASSERT_TRUE(nullptr == msgFilled);
}

/////////////////////////////////////////////////
TEST(FactoryTest, DeprecatedNonFullyQualified)
{
  auto msg = Factory::New("StringMsg");
  EXPECT_TRUE(msg.get() != nullptr);
}

/////////////////////////////////////////////////
TEST(FactoryTest, DynamicMessageNoEnv)
{
  // Message won't be found because descriptors aren't loaded
  auto msg = Factory::New("example.msgs.StringMsg");
  EXPECT_TRUE(msg.get() == nullptr);
}

/////////////////////////////////////////////////
TEST(FactoryTest, DynamicMessageWithEnv)
{
  std::string paths;
  std::filesystem::path test_path(kMsgsTestPath);
  paths += (test_path / "desc").string();
  paths += kEnvironmentVariableSeparator;
  paths += test_path.string();

  Factory::LoadDescriptors(paths);
  auto msg = Factory::New("example.msgs.StringMsg");
  EXPECT_TRUE(msg.get() != nullptr);
}

/////////////////////////////////////////////////
TEST(FactoryTest, NewAllRegisteredTypes)
{
  std::vector<std::string> types;
  Factory::Types(types);
  EXPECT_FALSE(types.empty());

  for (const auto &type : types)
  {
    auto msg = Factory::New(type);
    ASSERT_NE(nullptr, msg.get()) << type;
  }
}

/////////////////////////////////////////////////
TEST(FactoryTest, MultipleMessagesInAProto)
{
  auto typesInSameFile =
  {
    "gz.msgs.SerializedEntityMap",
    "gz.msgs.SerializedStateMap",
    "gz.msgs.SerializedStepMap"
  };

  std::vector<std::string> types;
  Factory::Types(types);
  EXPECT_FALSE(types.empty());

  for (const auto *type : typesInSameFile)
  {
    // Check all types are registered
    EXPECT_NE(std::find(types.begin(), types.end(), std::string(type)),
        types.end()) << type;

    // Check all types can be newed
    auto msg = Factory::New(type);
    EXPECT_NE(nullptr, msg.get()) << type;
  }

  // Compile-time check that pointer types exist
  {
    gz::msgs::SerializedEntityMapUniquePtr ptr{nullptr};
    EXPECT_EQ(nullptr, ptr);
  }
  {
    gz::msgs::ConstSerializedEntityMapUniquePtr ptr{nullptr};
    EXPECT_EQ(nullptr, ptr);
  }
  {
    gz::msgs::SerializedEntityMapSharedPtr ptr{nullptr};
    EXPECT_EQ(nullptr, ptr);
  }
  {
    gz::msgs::ConstSerializedEntityMapSharedPtr ptr{nullptr};
    EXPECT_EQ(nullptr, ptr);
  }

  {
    gz::msgs::SerializedStateMapUniquePtr ptr{nullptr};
    EXPECT_EQ(nullptr, ptr);
  }
  {
    gz::msgs::ConstSerializedStateMapUniquePtr ptr{nullptr};
    EXPECT_EQ(nullptr, ptr);
  }
  {
    gz::msgs::SerializedStateMapSharedPtr ptr{nullptr};
    EXPECT_EQ(nullptr, ptr);
  }
  {
    gz::msgs::ConstSerializedStateMapSharedPtr ptr{nullptr};
    EXPECT_EQ(nullptr, ptr);
  }

  {
    gz::msgs::SerializedStepMapUniquePtr ptr{nullptr};
    EXPECT_EQ(nullptr, ptr);
  }
  {
    gz::msgs::ConstSerializedStepMapUniquePtr ptr{nullptr};
    EXPECT_EQ(nullptr, ptr);
  }
  {
    gz::msgs::SerializedStepMapSharedPtr ptr{nullptr};
    EXPECT_EQ(nullptr, ptr);
  }
  {
    gz::msgs::ConstSerializedStepMapSharedPtr ptr{nullptr};
    EXPECT_EQ(nullptr, ptr);
  }
}

/////////////////////////////////////////////////
int main(int _argc, char **_argv)
{
  testing::InitGoogleTest(&_argc, _argv);
  return RUN_ALL_TESTS();
}
