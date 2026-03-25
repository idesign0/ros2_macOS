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

#include <iostream>
#include <string>
#include <vector>

#include "gz/transport/AdvertiseOptions.hh"
#include "gz/transport/Helpers.hh"
#include "gtest/gtest.h"

using namespace gz;
using namespace transport;

//////////////////////////////////////////////////
/// \brief Check the default constructor.
TEST(AdvertiseOptionsTest, defConstructor)
{
  AdvertiseOptions opts;
  EXPECT_EQ(opts.Scope(), Scope_t::ALL);
}

//////////////////////////////////////////////////
/// \brief Check the copy constructor.
TEST(AdvertiseOptionsTest, copyConstructor)
{
  AdvertiseOptions opts1;
  opts1.SetScope(Scope_t::HOST);
  AdvertiseOptions opts2(opts1);
  EXPECT_EQ(opts2.Scope(), opts1.Scope());
}

//////////////////////////////////////////////////
/// \brief Check the assignment operator.
TEST(AdvertiseOptionsTest, assignmentOp)
{
  AdvertiseOptions opts1;
  AdvertiseOptions opts2;
  opts1.SetScope(Scope_t::PROCESS);
  opts2 = opts1;
  EXPECT_EQ(opts2.Scope(), opts1.Scope());
}

//////////////////////////////////////////////////
/// \brief Check the == and != operators.
TEST(AdvertiseOptionsTest, equalityOp)
{
  AdvertiseOptions opts1;
  AdvertiseOptions opts2;
  opts1.SetScope(Scope_t::PROCESS);
  EXPECT_FALSE(opts1 == opts2);
  EXPECT_TRUE(opts1 != opts2);
  opts2.SetScope(Scope_t::PROCESS);
  EXPECT_TRUE(opts1 == opts2);
  EXPECT_FALSE(opts1 != opts2);
}

//////////////////////////////////////////////////
/// \brief Check the << operator
TEST(AdvertiseOptionsTest, streamInsertion)
{
  AdvertiseOptions opts;
  std::ostringstream output;
  output << opts;
  std::string expectedOutput =
    "Advertise options:\n"
    "\tScope: All\n";

  EXPECT_EQ(output.str(), expectedOutput);
}

//////////////////////////////////////////////////
/// \brief Check the accessors.
TEST(AdvertiseOptionsTest, accessors)
{
  // Scope.
  AdvertiseOptions opts;
  EXPECT_EQ(opts.Scope(), Scope_t::ALL);
  opts.SetScope(Scope_t::HOST);
  EXPECT_EQ(opts.Scope(), Scope_t::HOST);
}

//////////////////////////////////////////////////
/// \brief Check the default constructor.
TEST(AdvertiseOptionsTest, msgDefConstructor)
{
  AdvertiseMessageOptions opts;
  EXPECT_EQ(opts.Scope(), Scope_t::ALL);
  EXPECT_FALSE(opts.Throttled());
  EXPECT_EQ(opts.MsgsPerSec(), kUnthrottled);
}

//////////////////////////////////////////////////
/// \brief Check the copy constructor.
TEST(AdvertiseOptionsTest, msgCopyConstructor)
{
  AdvertiseMessageOptions opts1;
  opts1.SetScope(Scope_t::HOST);
  opts1.SetMsgsPerSec(10u);
  AdvertiseMessageOptions opts2(opts1);
  EXPECT_EQ(opts1, opts2);
}

//////////////////////////////////////////////////
/// \brief Check the assignment operator.
TEST(AdvertiseOptionsTest, msgAssignmentOp)
{
  AdvertiseMessageOptions opts1;
  AdvertiseMessageOptions opts2;
  opts1.SetScope(Scope_t::PROCESS);
  opts1.SetMsgsPerSec(10u);
  opts2 = opts1;
  EXPECT_EQ(opts1, opts2);
}

//////////////////////////////////////////////////
/// \brief Check the == and != operators.
TEST(AdvertiseOptionsTest, msgEqualityOp)
{
  AdvertiseMessageOptions opts1;
  AdvertiseMessageOptions opts2;
  opts1.SetScope(Scope_t::PROCESS);
  opts1.SetMsgsPerSec(10u);
  EXPECT_FALSE(opts1 == opts2);
  EXPECT_TRUE(opts1 != opts2);
  opts2.SetScope(Scope_t::PROCESS);
  opts2.SetMsgsPerSec(10u);
  EXPECT_TRUE(opts1 == opts2);
  EXPECT_FALSE(opts1 != opts2);
}

//////////////////////////////////////////////////
/// \brief Check the << operator
TEST(AdvertiseOptionsTest, msgStreamInsertion)
{
  AdvertiseMessageOptions opts;
  std::ostringstream output;
  output << opts;
  std::string expectedOutput =
    "Advertise options:\n"
    "\tScope: All\n"
    "\tThrottled? No\n";
  EXPECT_EQ(output.str(), expectedOutput);

  output.clear();
  output.str("");;
  opts.SetMsgsPerSec(10u);
  output << opts;
  expectedOutput =
    "Advertise options:\n"
    "\tScope: All\n"
    "\tThrottled? Yes\n"
    "\tRate: 10 msgs/sec\n";
  EXPECT_EQ(output.str(), expectedOutput);
}

//////////////////////////////////////////////////
/// \brief Check the accessors.
TEST(AdvertiseOptionsTest, msgAccessors)
{
  // Scope.
  AdvertiseMessageOptions opts;
  EXPECT_EQ(opts.Scope(), Scope_t::ALL);
  opts.SetScope(Scope_t::HOST);
  EXPECT_EQ(opts.Scope(), Scope_t::HOST);

  // MsgsPerSec
  EXPECT_FALSE(opts.Throttled());
  EXPECT_EQ(opts.MsgsPerSec(), kUnthrottled);
  opts.SetMsgsPerSec(10u);
  EXPECT_EQ(opts.MsgsPerSec(), 10u);
  EXPECT_TRUE(opts.Throttled());
}

//////////////////////////////////////////////////
/// \brief Check the default constructor.
TEST(AdvertiseOptionsTest, srvDefConstructor)
{
  AdvertiseServiceOptions opts;
  EXPECT_EQ(opts.Scope(), Scope_t::ALL);
}

//////////////////////////////////////////////////
/// \brief Check the copy constructor.
TEST(AdvertiseOptionsTest, srvCopyConstructor)
{
  AdvertiseServiceOptions opts1;
  opts1.SetScope(Scope_t::HOST);
  AdvertiseServiceOptions opts2(opts1);
  EXPECT_EQ(opts1, opts2);
}

//////////////////////////////////////////////////
/// \brief Check the assignment operator.
TEST(AdvertiseOptionsTest, srvAssignmentOp)
{
  AdvertiseServiceOptions opts1;
  AdvertiseServiceOptions opts2;
  opts1.SetScope(Scope_t::PROCESS);
  opts2 = opts1;
  EXPECT_EQ(opts1, opts2);
}

//////////////////////////////////////////////////
/// \brief Check the == and != operators.
TEST(AdvertiseOptionsTest, srvEqualityOp)
{
  AdvertiseServiceOptions opts1;
  AdvertiseServiceOptions opts2;
  opts1.SetScope(Scope_t::PROCESS);
  EXPECT_FALSE(opts1 == opts2);
  EXPECT_TRUE(opts1 != opts2);
  opts2.SetScope(Scope_t::PROCESS);
  EXPECT_TRUE(opts1 == opts2);
  EXPECT_FALSE(opts1 != opts2);
}

//////////////////////////////////////////////////
/// \brief Check the << operator
TEST(AdvertiseOptionsTest, srvStreamInsertion)
{
  AdvertiseServiceOptions opts;
  std::ostringstream output;
  output << opts;
  std::string expectedOutput =
    "Advertise options:\n"
    "\tScope: All\n";
  EXPECT_EQ(output.str(), expectedOutput);
}

//////////////////////////////////////////////////
/// \brief Check the accessors.
TEST(AdvertiseOptionsTest, srvAccessors)
{
  // Scope.
  AdvertiseServiceOptions opts;
  EXPECT_EQ(opts.Scope(), Scope_t::ALL);
  opts.SetScope(Scope_t::HOST);
  EXPECT_EQ(opts.Scope(), Scope_t::HOST);
}
