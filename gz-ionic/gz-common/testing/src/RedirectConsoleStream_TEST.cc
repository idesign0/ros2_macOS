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

#include "gz/common/Console.hh"
#include "gz/common/testing/RedirectConsoleStream.hh"
#include "gz/common/testing/TestPaths.hh"

using namespace gz::common;
using namespace gz::common::testing;

/////////////////////////////////////////////////
TEST(RedirectConsoleStream, RedirectConsoleStream)
{
  // Test an invalid configuration, where there redirect is not created
  auto tempDir = MakeTestTempDirectory();
  ASSERT_TRUE(tempDir->Valid());
  auto tmpOut = joinPaths(tempDir->Path(), "console_out");

  std::cout << "Before" << std::endl;
  auto redirect = RedirectConsoleStream(StreamSource::STDOUT, "");
  EXPECT_FALSE(redirect.Active());

  std::cout << "Foo" << std::endl;
  std::cout << "Bar" << std::endl;
  std::cout << "Baz" << std::endl;

  auto output = redirect.GetString();
  EXPECT_TRUE(output.empty());
  EXPECT_FALSE(redirect.Active());
  EXPECT_EQ(output.find("Before"), std::string::npos);
  EXPECT_EQ(output.find("After"), std::string::npos);
  EXPECT_EQ(output.find("Foo"), std::string::npos);
  EXPECT_EQ(output.find("Bar"), std::string::npos);
  EXPECT_EQ(output.find("Baz"), std::string::npos);

  std::cout << "After" << std::endl;
}

/////////////////////////////////////////////////
TEST(RedirectConsoleStream, RedirectStdout)
{
  const auto sourceType = StreamSource::STDOUT;
  auto& source = std::cout;

  auto tempDir = MakeTestTempDirectory();
  ASSERT_TRUE(tempDir->Valid());
  auto tmpOut = joinPaths(tempDir->Path(), "console_out");

  source << "Before" << std::endl;
  auto redirect = RedirectConsoleStream(sourceType, tmpOut);
  EXPECT_TRUE(redirect.Active());

  source << "Foo" << std::endl;
  source << "Bar" << std::endl;
  source << "Baz" << std::endl;

  {
    auto output = redirect.GetString();
    EXPECT_FALSE(redirect.Active());
    EXPECT_FALSE(output.empty());
    EXPECT_EQ(output.find("Before"), std::string::npos);
    EXPECT_EQ(output.find("After"), std::string::npos);
    EXPECT_NE(output.find("Foo"), std::string::npos);
    EXPECT_NE(output.find("Bar"), std::string::npos);
    EXPECT_NE(output.find("Baz"), std::string::npos);
  }

  source << "After" << std::endl;
  {
    auto output = redirect.GetString();
    EXPECT_FALSE(redirect.Active());
    EXPECT_FALSE(output.empty());
    EXPECT_EQ(output.find("Before"), std::string::npos);
    EXPECT_EQ(output.find("After"), std::string::npos);
    EXPECT_NE(output.find("Foo"), std::string::npos);
    EXPECT_NE(output.find("Bar"), std::string::npos);
    EXPECT_NE(output.find("Baz"), std::string::npos);
  }
}

/////////////////////////////////////////////////
TEST(RedirectConsoleStream, RedirectStderr)
{
  const auto sourceType = StreamSource::STDERR;
  auto& source = std::cerr;

  auto tempDir = MakeTestTempDirectory();
  ASSERT_TRUE(tempDir->Valid());
  auto tmpOut = joinPaths(tempDir->Path(), "console_out");

  source << "Before" << std::endl;
  auto redirect = RedirectConsoleStream(sourceType, tmpOut);
  EXPECT_TRUE(redirect.Active());

  source << "Foo" << std::endl;
  source << "Bar" << std::endl;
  source << "Baz" << std::endl;

  {
    auto output = redirect.GetString();
    EXPECT_FALSE(redirect.Active());
    EXPECT_FALSE(output.empty());
    EXPECT_EQ(output.find("Before"), std::string::npos);
    EXPECT_EQ(output.find("After"), std::string::npos);
    EXPECT_NE(output.find("Foo"), std::string::npos);
    EXPECT_NE(output.find("Bar"), std::string::npos);
    EXPECT_NE(output.find("Baz"), std::string::npos);
  }

  source << "After" << std::endl;
  {
    auto output = redirect.GetString();
    EXPECT_FALSE(redirect.Active());
    EXPECT_FALSE(output.empty());
    EXPECT_EQ(output.find("Before"), std::string::npos);
    EXPECT_EQ(output.find("After"), std::string::npos);
    EXPECT_NE(output.find("Foo"), std::string::npos);
    EXPECT_NE(output.find("Bar"), std::string::npos);
    EXPECT_NE(output.find("Baz"), std::string::npos);
  }
}

/////////////////////////////////////////////////
TEST(RedirectConsoleStream, InlineHelpers)
{
  auto redirOut = RedirectStdout();
  auto redirErr = RedirectStderr();

  std::cout << "Foo" << std::endl;
  std::cerr << "Bar" << std::endl;

  auto out = redirOut.GetString();
  auto err = redirErr.GetString();

  EXPECT_FALSE(out.empty());
  EXPECT_FALSE(err.empty());

  EXPECT_NE(out.find("Foo"), std::string::npos);
  EXPECT_EQ(err.find("Foo"), std::string::npos);

  EXPECT_EQ(out.find("Bar"), std::string::npos);
  EXPECT_NE(err.find("Bar"), std::string::npos);
}

/////////////////////////////////////////////////
TEST(RedirectConsoleStream, Console)
{
  auto redirOut = RedirectStdout();
  auto redirErr = RedirectStderr();

  gz::common::Console::SetVerbosity(4);

  gzerr << "error" << std::endl;
  gzwarn << "warning" << std::endl;
  gzmsg << "message" << std::endl;
  gzdbg << "debug" << std::endl;

  auto out = redirOut.GetString();
  auto err = redirErr.GetString();

  EXPECT_FALSE(out.empty());
  EXPECT_FALSE(err.empty());

  EXPECT_EQ(out.find("error"), std::string::npos);
  EXPECT_EQ(out.find("warning"), std::string::npos);
  EXPECT_NE(out.find("message"), std::string::npos);
  EXPECT_NE(out.find("debug"), std::string::npos);

  EXPECT_NE(err.find("error"), std::string::npos);
  EXPECT_NE(err.find("warning"), std::string::npos);
  EXPECT_EQ(err.find("message"), std::string::npos);
  EXPECT_EQ(err.find("debug"), std::string::npos);
}
