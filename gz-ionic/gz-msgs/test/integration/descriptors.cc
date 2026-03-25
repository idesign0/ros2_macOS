/*
 * Copyright (C) 2024 Open Source Robotics Foundation
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

#include <filesystem>

#include "gz/msgs/Factory.hh"

static constexpr const char * kMsgsTestPath = GZ_MSGS_TEST_PATH;

TEST(FactoryTest, DynamicFactory)
{
  EXPECT_EQ(nullptr, gz::msgs::Factory::New("example.msgs.StringMsg"));
  EXPECT_EQ(nullptr, gz::msgs::Factory::New("testing.Bytes"));
  EXPECT_EQ(nullptr, gz::msgs::Factory::New("testing.FooMessage"));
  EXPECT_EQ(nullptr, gz::msgs::Factory::New("testing.BarMessage"));
  EXPECT_EQ(nullptr, gz::msgs::Factory::New("testing.BazMessage"));

  // Test loading an invalid path
  {
    std::filesystem::path test_path(kMsgsTestPath);
    std::string paths = (test_path / "desc" / "does_not_exist.desc").string();
    gz::msgs::Factory::LoadDescriptors(paths);
  }
  EXPECT_EQ(nullptr, gz::msgs::Factory::New("example.msgs.StringMsg"));
  EXPECT_EQ(nullptr, gz::msgs::Factory::New("testing.Bytes"));
  EXPECT_EQ(nullptr, gz::msgs::Factory::New("testing.FooMessage"));
  EXPECT_EQ(nullptr, gz::msgs::Factory::New("testing.BarMessage"));
  EXPECT_EQ(nullptr, gz::msgs::Factory::New("testing.BazMessage"));

  // Test loading an incorrect extension
  {
    std::filesystem::path test_path(kMsgsTestPath);
    std::string paths = (test_path / "desc" / "testing.invalid").string();
    gz::msgs::Factory::LoadDescriptors(paths);
  }
  EXPECT_EQ(nullptr, gz::msgs::Factory::New("example.msgs.StringMsg"));
  EXPECT_EQ(nullptr, gz::msgs::Factory::New("testing.Bytes"));
  EXPECT_EQ(nullptr, gz::msgs::Factory::New("testing.FooMessage"));
  EXPECT_EQ(nullptr, gz::msgs::Factory::New("testing.BarMessage"));
  EXPECT_EQ(nullptr, gz::msgs::Factory::New("testing.BazMessage"));

  // Test loading a file with invalid content
  {
    std::filesystem::path test_path(kMsgsTestPath);
    std::string paths = (test_path / "desc" / "testing.proto").string();
    gz::msgs::Factory::LoadDescriptors(paths);
  }
  EXPECT_EQ(nullptr, gz::msgs::Factory::New("example.msgs.StringMsg"));
  EXPECT_EQ(nullptr, gz::msgs::Factory::New("testing.BarBytes"));
  EXPECT_EQ(nullptr, gz::msgs::Factory::New("testing.FooMessage"));
  EXPECT_EQ(nullptr, gz::msgs::Factory::New("testing.BarMessage"));
  EXPECT_EQ(nullptr, gz::msgs::Factory::New("testing.BazMessage"));

  // Load a valid descriptor file with one message type
  {
    std::filesystem::path test_path(kMsgsTestPath);
    std::string paths = (test_path / "desc" / "stringmsg.desc").string();
    gz::msgs::Factory::LoadDescriptors(paths);
  }

  EXPECT_NE(nullptr, gz::msgs::Factory::New("example.msgs.StringMsg"));
  EXPECT_EQ(nullptr, gz::msgs::Factory::New("testing.Bytes"));
  EXPECT_EQ(nullptr, gz::msgs::Factory::New("testing.FooMessage"));
  EXPECT_EQ(nullptr, gz::msgs::Factory::New("testing.BarMessage"));
  EXPECT_EQ(nullptr, gz::msgs::Factory::New("testing.BazMessage"));

  // Load a directory
  {
    std::filesystem::path test_path(kMsgsTestPath);
    std::string paths = (test_path / "desc").string();
    gz::msgs::Factory::LoadDescriptors(paths);
  }

  EXPECT_NE(nullptr, gz::msgs::Factory::New("example.msgs.StringMsg"));
  EXPECT_NE(nullptr, gz::msgs::Factory::New("testing.FooBytes"));
  EXPECT_NE(nullptr, gz::msgs::Factory::New("testing.FooMessage"));
  EXPECT_NE(nullptr, gz::msgs::Factory::New("testing.BarMessage"));
  EXPECT_NE(nullptr, gz::msgs::Factory::New("testing.BazMessage"));
}
