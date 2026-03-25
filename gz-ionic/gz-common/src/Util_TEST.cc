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

#include <fstream>
#include <stdlib.h>
#include <string>

#include <gz/common/Util.hh>

using namespace gz;

/////////////////////////////////////////////////
/// \brief Test Util::SHA1
TEST(Util_TEST, SHA1)
{
  // Do not forget to update 'precomputedSHA1' if you modify the SHA1 input.
  std::string precomputedSHA1;
  std::string computedSHA1;
  std::string s;

  // Compute the SHA1 of the vector
  std::vector<float> v;
  for (int i = 0; i < 100; ++i)
    v.push_back(static_cast<float>(i));

  computedSHA1 = common::sha1<std::vector<float> >(v);
  precomputedSHA1 = "913283ec8502ba1423d38a7ea62cb8e492e87b23";
  EXPECT_EQ(precomputedSHA1, computedSHA1);

  // Compute the SHA1 of a string
  s = "Marty McFly: Wait a minute, Doc. Ah... Are you telling me that you"
      " built a time machine... out of a DeLorean?\n"
      "Dr. Emmett Brown: The way I see it, if you're gonna build a time"
      " machine into a car, why not do it with some style?";
  computedSHA1 = common::sha1<std::string>(s);
  precomputedSHA1 = "a370ddc4d61d936b2bb40f98bae061dc15fd8923";
  EXPECT_EQ(precomputedSHA1, computedSHA1);

  // Compute the SHA1 of an empty string
  s = "";
  computedSHA1 = common::sha1<std::string>(s);
  precomputedSHA1 = "da39a3ee5e6b4b0d3255bfef95601890afd80709";
  EXPECT_EQ(precomputedSHA1, computedSHA1);

  // Compute a bunch of SHA1's to verify consistent length
  for (unsigned i = 0; i < 100; ++i)
  {
    std::stringstream stream;
    stream << i << '\n';
    std::string sha = common::sha1<std::string>(stream.str());
    EXPECT_EQ(sha.length(), 40u);
  }
}

/////////////////////////////////////////////////
/// \brief Test Util::hash64
TEST(Util_TEST, Hash64)
{
  // Do not forget to update 'precomputedHash' if you modify the string input.
  {
    constexpr auto computedHash = common::hash64(
      "Look at this. It’s worthless — ten dollars from a vendor in the street. "
      "But I take it, I bury it in the sand for a thousand years, it becomes "
      "priceless. Like the Ark.");
    uint64_t precomputedHash = 5217348260923471204U;
    EXPECT_EQ(precomputedHash, computedHash);
  }

  // Compute the hash64 of an empty string.
  {
    constexpr auto computedHash = common::hash64("");
    uint64_t precomputedHash = 14695981039346656037U;
    EXPECT_EQ(precomputedHash, computedHash);
  }
}

/////////////////////////////////////////////////
/// \brief Test the string tokenizer split() function.
TEST(Util_TEST, split)
{
  auto tokens = common::split("abc/def", "/");
  ASSERT_EQ(tokens.size(), 2u);
  EXPECT_EQ(tokens.at(0), "abc");
  EXPECT_EQ(tokens.at(1), "def");

  tokens = common::split("abc/def/", "/");
  ASSERT_EQ(tokens.size(), 2u);
  EXPECT_EQ(tokens.at(0), "abc");
  EXPECT_EQ(tokens.at(1), "def");

  tokens = common::split("//abc/def///", "/");
  ASSERT_EQ(tokens.size(), 2u);
  EXPECT_EQ(tokens.at(0), "abc");
  EXPECT_EQ(tokens.at(1), "def");

  tokens = common::split("abc", "/");
  ASSERT_EQ(tokens.size(), 1u);
  EXPECT_EQ(tokens.at(0), "abc");

  tokens = common::split("//abc/def::123::567///", "/");
  ASSERT_EQ(tokens.size(), 2u);
  EXPECT_EQ(tokens.at(0), "abc");
  EXPECT_EQ(tokens.at(1), "def::123::567");
  tokens = common::split("//abc/def::123::567///", "::");
  ASSERT_EQ(tokens.size(), 3u);
  EXPECT_EQ(tokens.at(0), "//abc/def");
  EXPECT_EQ(tokens.at(1), "123");
  EXPECT_EQ(tokens.at(2), "567///");

  tokens = common::split("12\r\n34\r\n56", "\r\n");
  ASSERT_EQ(tokens.size(), 3u);
  EXPECT_EQ(tokens.at(0), "12");
  EXPECT_EQ(tokens.at(1), "34");
  EXPECT_EQ(tokens.at(2), "56");
}

/////////////////////////////////////////////////
TEST(Util_TEST, replaceAll)
{
  std::string orig = "//abcd/efg///ijk////lm/////////////nop//";

  // No change should occur
  std::string result = common::replaceAll(orig, "//", "//");
  EXPECT_EQ(result, orig);
  result = common::replaceAll(orig, "/", "/");
  EXPECT_EQ(result, orig);

  result = common::replaceAll(orig, "//", "::");
  EXPECT_EQ(result, "::abcd/efg::/ijk::::lm::::::::::::/nop::");

  result = common::replaceAll(result, "a", "aaaa");
  EXPECT_EQ(result, "::aaaabcd/efg::/ijk::::lm::::::::::::/nop::");

  result = common::replaceAll(result, "::aaaa", " ");
  EXPECT_EQ(result, " bcd/efg::/ijk::::lm::::::::::::/nop::");

  result = common::replaceAll(result, " ", "_");
  EXPECT_EQ(result, "_bcd/efg::/ijk::::lm::::::::::::/nop::");

  std::string spaces = " 1  2   3    4     5      6       7 ";
  result = common::replaceAll(spaces, " ", "_");
  EXPECT_EQ(result, "_1__2___3____4_____5______6_______7_");

  result = common::replaceAll(spaces, "  ", "_");
  EXPECT_EQ(result, " 1_2_ 3__4__ 5___6___ 7 ");

  std::string test = "12345555675";
  common::replaceAll(test, test, "5", "*");
  EXPECT_EQ(test, "1234****67*");
}

/////////////////////////////////////////////////
TEST(Util_TEST, emptyENV)
{
  std::string var;
  EXPECT_FALSE(common::env("!!SHOULD_NOT_EXIST!!", var));
  EXPECT_TRUE(var.empty());
}

/////////////////////////////////////////////////
TEST(Util_TEST, envSet)
{
  const auto key = "GZ_ENV_SET";
  ASSERT_TRUE(common::setenv(key, "VALUE"));

  // Check set var
  {
    std::string value;
    EXPECT_TRUE(common::env(key, value));
    EXPECT_FALSE(value.empty());
    EXPECT_EQ("VALUE", value);
  }

  // Check set var with allowEmpty
  {
    std::string value;
    EXPECT_TRUE(common::env(key, value, true));
    EXPECT_FALSE(value.empty());
    EXPECT_EQ("VALUE", value);
  }

  // Check set var without allowEmpty
  {
    std::string value;
    EXPECT_TRUE(common::env(key, value, false));
    EXPECT_FALSE(value.empty());
    EXPECT_EQ("VALUE", value);
  }

  ASSERT_TRUE(common::unsetenv(key));
}

/////////////////////////////////////////////////
TEST(Util_TEST, envUnset)
{
  const auto key = "GZ_ENV_UNSET";
  ASSERT_TRUE(common::unsetenv(key));

  // Check unset var (default)
  {
    std::string value;
    EXPECT_FALSE(common::env(key, value));
    EXPECT_TRUE(value.empty());
  }

  // Check unset var with allowEmpty
  {
    std::string value;
    EXPECT_FALSE(common::env(key, value, true));
    EXPECT_TRUE(value.empty());
  }

  // Check unset var without allowEmpty
  {
    std::string value;
    EXPECT_FALSE(common::env(key, value, false));
    EXPECT_TRUE(value.empty());
  }
  ASSERT_TRUE(common::unsetenv(key));
}

/////////////////////////////////////////////////
TEST(Util_TEST, envSetEmpty)
{
  const auto key = "GZ_ENV_SET_EMPTY";

  ASSERT_TRUE(common::setenv(key, ""));
  ASSERT_FALSE(common::setenv("", ""));

  // Check set empty var (default)
  {
    std::string value;
    EXPECT_FALSE(common::env(key, value));
    EXPECT_TRUE(value.empty());
  }

#ifdef _WIN32
  {
    // This will warn on Windows, but return false
    std::string value;
    EXPECT_FALSE(common::env(key, value, true));
    EXPECT_TRUE(value.empty());
  }
#else
  {
    // This will not warn and return true on Linux,
    // as empty environment variables are allowed.
    std::string value;
    EXPECT_TRUE(common::env(key, value, true));
    EXPECT_TRUE(value.empty());
  }
#endif

  // Check set empty var without allowEmpty
  {
    std::string value;
    EXPECT_FALSE(common::env(key, value, false));
    EXPECT_TRUE(value.empty());
  }
  ASSERT_TRUE(common::unsetenv(key));
}

/////////////////////////////////////////////////
TEST(Util_TEST, findFile)
{
  EXPECT_EQ("", common::findFile("no_such_file"));

  // Create file to find
  auto dir1 = common::absPath("test_dir1");
  common::createDirectories(dir1);
  auto file1 = common::absPath(
      common::joinPaths(dir1, "test_f1"));

  std::ofstream fout;
  fout.open(file1, std::ofstream::out);
  fout << "asdf";
  fout.close();

  // Relative path
  auto systemPaths = common::systemPaths();
  ASSERT_NE(nullptr, systemPaths);
  EXPECT_EQ(file1, systemPaths->FindFile(common::joinPaths("test_dir1",
      "test_f1"), true));

  // Relative prefixed by file:
  EXPECT_EQ(file1, common::findFile("file:test_dir1/test_f1"));

  // Custom callback
#ifndef _WIN32
  const auto tmpDir = "/tmp";
#else
  const auto tmpDir = "C:\\Windows";
#endif
  auto fileCb = [&tmpDir](const common::URI &)
  {
    return tmpDir;
  };

  common::addFindFileURICallback(fileCb);
  EXPECT_EQ(tmpDir, common::findFile("model://banana"));
}

/////////////////////////////////////////////////
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
