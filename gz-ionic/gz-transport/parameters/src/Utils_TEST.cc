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

#include "Utils.hh"

#include <gz/msgs/boolean.pb.h>
#include <gz/msgs/stringmsg.pb.h>

#include "gtest/gtest.h"

using namespace gz;
using namespace transport;
using namespace transport::parameters;

//////////////////////////////////////////////////
TEST(ParametersUtils, addGzMsgsPrefix)
{
  EXPECT_EQ(addGzMsgsPrefix("asd"), "gz_msgs.asd");
}

//////////////////////////////////////////////////
TEST(ParametersUtils, getGzTypeFromAnyProto)
{
  google::protobuf::Any any;

  gz::msgs::Boolean boolMsg;
#if PROTOBUF_VERSION < 3015000
  any.PackFrom(boolMsg);
#else
  EXPECT_TRUE(any.PackFrom(boolMsg));
#endif
  EXPECT_EQ(*getGzTypeFromAnyProto(any), "Boolean");
  gz::msgs::StringMsg strMsg;
#if PROTOBUF_VERSION < 3015000
  any.PackFrom(strMsg);
#else
  EXPECT_TRUE(any.PackFrom(strMsg));
#endif
  EXPECT_EQ(*getGzTypeFromAnyProto(any), "StringMsg");
}
