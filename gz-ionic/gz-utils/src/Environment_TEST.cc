/*
 * Copyright (C) 2021 Open Source Robotics Foundation
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

#include <gz/utils/Environment.hh>

#include <string>

using namespace gz;

/////////////////////////////////////////////////
TEST(Environment, emptyENV)
{
  gz::utils::clearenv();

  std::string var;
  EXPECT_FALSE(utils::env("!!SHOULD_NOT_EXIST!!", var));
  EXPECT_TRUE(var.empty());
}

/////////////////////////////////////////////////
TEST(Environment, envSet)
{
  gz::utils::clearenv();

  const auto key = "GZ_ENV_SET";
  ASSERT_TRUE(utils::setenv(key, "VALUE"));

  // Check set var
  {
    std::string value;
    EXPECT_TRUE(utils::env(key, value));
    EXPECT_FALSE(value.empty());
    EXPECT_EQ("VALUE", value);
  }

  // Check set var with allowEmpty
  {
    std::string value;
    EXPECT_TRUE(utils::env(key, value, true));
    EXPECT_FALSE(value.empty());
    EXPECT_EQ("VALUE", value);
  }

  // Check set var without allowEmpty
  {
    std::string value;
    EXPECT_TRUE(utils::env(key, value, false));
    EXPECT_FALSE(value.empty());
    EXPECT_EQ("VALUE", value);
  }

  ASSERT_TRUE(utils::unsetenv(key));
}

/////////////////////////////////////////////////
TEST(Environment, envUnset)
{
  gz::utils::clearenv();

  const auto key = "GZ_ENV_UNSET";
  ASSERT_TRUE(utils::unsetenv(key));

  // Check unset var (default)
  {
    std::string value;
    EXPECT_FALSE(utils::env(key, value));
    EXPECT_TRUE(value.empty());
  }

  // Check unset var with allowEmpty
  {
    std::string value;
    EXPECT_FALSE(utils::env(key, value, true));
    EXPECT_TRUE(value.empty());
  }

  // Check unset var without allowEmpty
  {
    std::string value;
    EXPECT_FALSE(utils::env(key, value, false));
    EXPECT_TRUE(value.empty());
  }
  ASSERT_TRUE(utils::unsetenv(key));
}

/////////////////////////////////////////////////
TEST(Environment, envSetEmpty)
{
  gz::utils::clearenv();

  const auto key = "GZ_ENV_SET_EMPTY";

  ASSERT_TRUE(utils::setenv(key, ""));
  ASSERT_FALSE(utils::setenv("", ""));

  // Check set empty var (default)
  {
    std::string value;
    EXPECT_FALSE(utils::env(key, value));
    EXPECT_TRUE(value.empty());
  }

#ifdef _WIN32
  {
    // This will warn on Windows, but return false
    std::string value;
    EXPECT_FALSE(utils::env(key, value, true));
    EXPECT_TRUE(value.empty());
  }
#else
  {
    // This will not warn and return true on Linux,
    // as empty environment variables are allowed.
    std::string value;
    EXPECT_TRUE(utils::env(key, value, true));
    EXPECT_TRUE(value.empty());
  }
#endif

  // Check set empty var without allowEmpty
  {
    std::string value;
    EXPECT_FALSE(utils::env(key, value, false));
    EXPECT_TRUE(value.empty());
  }
  ASSERT_TRUE(utils::unsetenv(key));
}

/////////////////////////////////////////////////
TEST(Environment, envGetCollection)
{
  gz::utils::clearenv();
  auto currentEnv = gz::utils::env();
  EXPECT_EQ(currentEnv.size(), 0);

  ASSERT_TRUE(gz::utils::setenv("GZ_FOO_KEY", "GZ_FOO_VAL"));
  ASSERT_TRUE(gz::utils::setenv("GZ_BAR_KEY", "GZ_BAR_VAL"));
  ASSERT_TRUE(gz::utils::setenv("GZ_BAZ_KEY", "GZ_BAZ_VAL"));

  currentEnv = gz::utils::env();
  EXPECT_EQ(currentEnv.size(), 3);

  EXPECT_EQ(currentEnv["GZ_FOO_KEY"], "GZ_FOO_VAL");
  EXPECT_EQ(currentEnv["GZ_BAR_KEY"], "GZ_BAR_VAL");
  EXPECT_EQ(currentEnv["GZ_BAZ_KEY"], "GZ_BAZ_VAL");
}

/////////////////////////////////////////////////
TEST(Environment, printenv)
{
  gz::utils::clearenv();
  EXPECT_EQ(gz::utils::printenv(), "");

  ASSERT_TRUE(gz::utils::setenv("GZ_FOO_KEY", "GZ_FOO_VAL"));
  ASSERT_TRUE(gz::utils::setenv("GZ_BAR_KEY", "GZ_BAR_VAL"));
  ASSERT_TRUE(gz::utils::setenv("GZ_BAZ_KEY", "GZ_BAZ_VAL"));

  // Always returned in sorted order
  EXPECT_EQ(gz::utils::printenv(),
    "GZ_BAR_KEY=GZ_BAR_VAL\nGZ_BAZ_KEY=GZ_BAZ_VAL\nGZ_FOO_KEY=GZ_FOO_VAL\n");
}

/////////////////////////////////////////////////
TEST(Environment, envStringsToMap)
{
  gz::utils::EnvironmentStrings strings;
  strings.emplace_back("GZ_FOO_KEY=GZ_FOO_VAL");
  strings.emplace_back("GZ_BAR_KEY=GZ_BAR_VAL");
  strings.emplace_back("GZ_BAZ_KEY=GZ_BAZ_VAL");
  strings.emplace_back("BAD_KEY");

  {
    auto envMap = gz::utils::envStringsToMap(strings);
    EXPECT_EQ(3u, envMap.size());
    EXPECT_EQ("GZ_FOO_VAL", envMap["GZ_FOO_KEY"]);
    EXPECT_EQ("GZ_BAR_VAL", envMap["GZ_BAR_KEY"]);
    EXPECT_EQ("GZ_BAZ_VAL", envMap["GZ_BAZ_KEY"]);
  }
}

/////////////////////////////////////////////////
TEST(Environment, envMapToStrings)
{
  gz::utils::EnvironmentMap envMap;
  envMap.insert({{"GZ_FOO_KEY"}, {"GZ_FOO_VAL"}});
  envMap.insert({{"GZ_BAR_KEY"}, {"GZ_BAR_VAL"}});
  envMap.insert({{"GZ_BAZ_KEY"}, {"GZ_BAZ_VAL"}});

  {
    auto envStrings = gz::utils::envMapToStrings(envMap);

    EXPECT_EQ(3u, envStrings.size());
    EXPECT_EQ("GZ_BAR_KEY=GZ_BAR_VAL", envStrings[0]);
    EXPECT_EQ("GZ_BAZ_KEY=GZ_BAZ_VAL", envStrings[1]);
    EXPECT_EQ("GZ_FOO_KEY=GZ_FOO_VAL", envStrings[2]);
  }
}
