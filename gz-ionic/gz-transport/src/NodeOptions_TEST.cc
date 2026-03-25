/*
 * Copyright (C) 2015 Open Source Robotics Foundation
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

#include "gtest/gtest.h"

#include <string>

#include "gz/transport/NetUtils.hh"
#include "gz/transport/NodeOptions.hh"

#include <gz/utils/Environment.hh>

using namespace gz;

//////////////////////////////////////////////////
/// \brief Check that  is set.
TEST(NodeOptionsTest, ignPartition)
{
  // Set GZ_PARTITION
  std::string aPartition = "customPartition";
  ASSERT_TRUE(gz::utils::setenv("GZ_PARTITION", aPartition));

  transport::NodeOptions opts;
  EXPECT_EQ(opts.Partition(), aPartition);

  // A partition set by the user should overwrite GZ_PARTITION.
  std::string userPartition = "userPartition";
  opts.SetPartition(userPartition);
  EXPECT_EQ(opts.Partition(), userPartition);

  // Copy constructor
  transport::NodeOptions opts2(opts);
  EXPECT_EQ(opts.Partition(), opts2.Partition());
  EXPECT_EQ(opts.NameSpace(), opts2.NameSpace());

  EXPECT_FALSE(opts2.SetPartition("/"));
}

//////////////////////////////////////////////////
/// \brief Check the accessors.
TEST(NodeOptionsTest, accessors)
{
  // Check the default values.
  gz::utils::unsetenv("GZ_PARTITION");
  transport::NodeOptions opts;
  EXPECT_TRUE(opts.NameSpace().empty());
  auto defaultPartition = transport::hostname() + ":" + transport::username();
  EXPECT_EQ(opts.Partition(), defaultPartition);

  // NameSpace.
  std::string aNamespace = "validNamespace";
  EXPECT_FALSE(opts.SetNameSpace("invalid namespace"));
  EXPECT_EQ(opts.NameSpace(), "");
  EXPECT_TRUE(opts.SetNameSpace(aNamespace));
  EXPECT_EQ(opts.NameSpace(), aNamespace);

  // Partition.
  std::string aPartition = "validPartition";
  EXPECT_FALSE(opts.SetPartition("invalid partition"));
  EXPECT_EQ(opts.Partition(), defaultPartition);
  EXPECT_TRUE(opts.SetPartition(aPartition));
  EXPECT_EQ(opts.Partition(), aPartition);
}
