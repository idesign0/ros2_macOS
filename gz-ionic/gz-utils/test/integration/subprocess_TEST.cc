/*
 * Copyright (C) 2023 Open Source Robotics Foundation
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

#include <gz/utils/Subprocess.hh>

#include <chrono>
#include <thread>
#include "gz/utils/Environment.hh"

using Subprocess = gz::utils::Subprocess;

static constexpr const char * kExecutablePath = SUBPROCESS_EXECUTABLE_PATH;

/////////////////////////////////////////////////
TEST(Subprocess, CreateInvalid)
{
  auto proc = Subprocess({"an_executable_that_will_never_exist"});
  EXPECT_FALSE(proc.Alive());
}

/////////////////////////////////////////////////
TEST(Subprocess, CreateInvalidSpaces)
{
  // Test if a user passes a string with spaces, rather than
  // a vector of strings
  std::string command(kExecutablePath);
  command.append(" --help");
  auto proc = Subprocess({command});

  // Block until the executable is done
  auto ret = proc.Join();
  EXPECT_EQ(-1, ret);

  EXPECT_FALSE(proc.Alive());

  auto cout = proc.Stdout();
  auto cerr = proc.Stdout();

  EXPECT_TRUE(cout.empty()) << cout;
  EXPECT_TRUE(cerr.empty()) << cerr;
}

/////////////////////////////////////////////////
TEST(Subprocess, CreateValid)
{
  auto proc = Subprocess({kExecutablePath, "--help"});

  // Block until the executable is done
  auto ret = proc.Join();
  EXPECT_EQ(0u, ret);

  EXPECT_FALSE(proc.Alive());

  auto cout = proc.Stdout();
  auto cerr = proc.Stdout();

  EXPECT_FALSE(cout.empty());
  EXPECT_TRUE(cerr.empty()) << cerr;
}

/////////////////////////////////////////////////
TEST(Subprocess, Cout)
{
  auto proc = Subprocess({kExecutablePath,
                          "--output=cout",
                          "--iterations=10",
                          "--iteration-ms=10"});

  EXPECT_TRUE(proc.Alive());

  // Block until the executable is done
  auto ret = proc.Join();
  EXPECT_EQ(0u, ret);

  auto cout = proc.Stdout();
  auto cerr = proc.Stderr();
  EXPECT_FALSE(cout.empty());
  EXPECT_NE(std::string::npos, cout.find("Iteration: ")) << cout;
  EXPECT_TRUE(cerr.empty()) << cerr;
}

/////////////////////////////////////////////////
TEST(Subprocess, Cerr)
{
  auto proc = Subprocess({kExecutablePath,
                          "--output=cerr",
                          "--iterations=10",
                          "--iteration-ms=10"});
  EXPECT_TRUE(proc.Alive());

  // Block until the executable is done
  auto ret = proc.Join();
  EXPECT_EQ(0u, ret);

  EXPECT_FALSE(proc.Alive());
  auto cout = proc.Stdout();
  auto cerr = proc.Stderr();
  EXPECT_TRUE(cout.empty()) << cout;
  EXPECT_FALSE(cerr.empty());
  EXPECT_NE(std::string::npos, cerr.find("Iteration: ")) << cerr;
}

/////////////////////////////////////////////////
TEST(Subprocess, Environment)
{
  ASSERT_TRUE(gz::utils::clearenv());
  ASSERT_TRUE(gz::utils::setenv("GZ_FOO_KEY", "GZ_FOO_VAL"));
  ASSERT_TRUE(gz::utils::setenv("GZ_BAR_KEY", "GZ_BAR_VAL"));
  ASSERT_TRUE(gz::utils::setenv("GZ_BAZ_KEY", "GZ_BAZ_VAL"));

  {
    // Default behavior is to inherit the environment
    auto proc = Subprocess({kExecutablePath, "--output=cout", "--environment"});
    // Block until the executable is done
    auto ret = proc.Join();
    EXPECT_EQ(0u, ret);

    auto cout = proc.Stdout();
    EXPECT_NE(std::string::npos, cout.find("GZ_FOO_KEY=GZ_FOO_VAL"));
    EXPECT_NE(std::string::npos, cout.find("GZ_BAR_KEY=GZ_BAR_VAL"));
    EXPECT_NE(std::string::npos, cout.find("GZ_BAZ_KEY=GZ_BAZ_VAL"));
  }

  {
    // Passing an empty map as the second arg clears the environment
    auto proc = Subprocess(
      {kExecutablePath, "--output=cout", "--environment"},
      gz::utils::EnvironmentMap());
    // Block until the executable is done
    auto ret = proc.Join();
    EXPECT_EQ(0u, ret);

    auto cout = proc.Stdout();
    EXPECT_EQ(std::string::npos, cout.find("GZ_FOO_KEY=GZ_FOO_VAL"));
    EXPECT_EQ(std::string::npos, cout.find("GZ_BAR_KEY=GZ_BAR_VAL"));
    EXPECT_EQ(std::string::npos, cout.find("GZ_BAZ_KEY=GZ_BAZ_VAL"));
  }

  {
    // Passing an empty vector as the second arg clears the environment
    auto proc = Subprocess(
      {kExecutablePath, "--output=cout", "--environment"},
      gz::utils::EnvironmentStrings());
    // Block until the executable is done
    auto ret = proc.Join();
    EXPECT_EQ(0u, ret);

    auto cout = proc.Stdout();
    EXPECT_EQ(std::string::npos, cout.find("GZ_FOO_KEY=GZ_FOO_VAL"));
    EXPECT_EQ(std::string::npos, cout.find("GZ_BAR_KEY=GZ_BAR_VAL"));
    EXPECT_EQ(std::string::npos, cout.find("GZ_BAZ_KEY=GZ_BAZ_VAL"));
  }

  {
    // Passing a map sets those variables, clearing the rest
    auto proc = Subprocess(
      {kExecutablePath, "--output=cout", "--environment"},
      gz::utils::EnvironmentMap{
        {"QUX_KEY", "QUX_VAL"}
      });
    // Block until the executable is done
    auto ret = proc.Join();
    EXPECT_EQ(0u, ret);

    auto cout = proc.Stdout();
    EXPECT_EQ(std::string::npos, cout.find("GZ_FOO_KEY=GZ_FOO_VAL"));
    EXPECT_EQ(std::string::npos, cout.find("GZ_BAR_KEY=GZ_BAR_VAL"));
    EXPECT_EQ(std::string::npos, cout.find("GZ_BAZ_KEY=GZ_BAZ_VAL"));
    EXPECT_NE(std::string::npos, cout.find("QUX_KEY=QUX_VAL"));
  }

  {
    // Passing a map sets those variables, clearing the rest
    auto proc = Subprocess(
      {kExecutablePath, "--output=cout", "--environment"},
      gz::utils::EnvironmentStrings{
        {"QUX_KEY=QUX_VAL"}
      });
    // Block until the executable is done
    auto ret = proc.Join();
    EXPECT_EQ(0u, ret);

    auto cout = proc.Stdout();
    EXPECT_EQ(std::string::npos, cout.find("GZ_FOO_KEY=GZ_FOO_VAL"));
    EXPECT_EQ(std::string::npos, cout.find("GZ_BAR_KEY=GZ_BAR_VAL"));
    EXPECT_EQ(std::string::npos, cout.find("GZ_BAZ_KEY=GZ_BAZ_VAL"));
    EXPECT_NE(std::string::npos, cout.find("QUX_KEY=QUX_VAL"));
  }

}
