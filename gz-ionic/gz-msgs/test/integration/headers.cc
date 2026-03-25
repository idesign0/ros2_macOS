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

#include "gz/msgs/contact.pb.h"

/////////////////////////////////////////////////
TEST(Messages, NestedHeaders)
{
  // Empty contact
  gz::msgs::Contact contact;
  std::string contactSerialized;
  EXPECT_TRUE(contact.SerializeToString(&contactSerialized));
  EXPECT_EQ(0u, contactSerialized.size());

  // Empty wrench
  gz::msgs::JointWrench wrench;
  std::string wrenchSerialized;
  EXPECT_TRUE(wrench.SerializeToString(&wrenchSerialized));
  EXPECT_EQ(0u, wrenchSerialized.size());

  // Add header data to the wrench message
  wrench.mutable_header()->mutable_stamp()->set_sec(123);
  EXPECT_TRUE(wrench.SerializeToString(&wrenchSerialized));
  EXPECT_EQ(6u, wrenchSerialized.size());

  // Set the same header data on contact's embedded wrench message
  contact.add_wrench()->mutable_header()->mutable_stamp()->set_sec(123);
  EXPECT_TRUE(contact.SerializeToString(&contactSerialized));

  // The contact message should be a bit larger than the standalone wrench
  // message since the contact message needs to contain information about the
  // existence of its embedded wrench message.
  EXPECT_EQ(8u, contactSerialized.size());

  // The contact's embedded wrench message should be the same size as the
  // standalone wrench message.
  EXPECT_TRUE(contact.wrench(0).SerializeToString(&contactSerialized));
  EXPECT_EQ(wrenchSerialized.size(), contactSerialized.size());
}
