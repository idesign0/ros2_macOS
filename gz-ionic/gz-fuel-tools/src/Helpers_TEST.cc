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
#include <gz/common/Filesystem.hh>
#include "gz/fuel_tools/Helpers.hh"

using namespace gz;
using namespace fuel_tools;

/////////////////////////////////////////////////
TEST(HelpersTEST, SanitizeAuthority)
{
  EXPECT_EQ("localhost%3A8000", sanitizeAuthority("localhost:8000"));
  EXPECT_EQ("localhost%3A8001", sanitizeAuthority("localhost:8001"));
  EXPECT_EQ("localhost%3A%3A8002", sanitizeAuthority("localhost::8002"));
  EXPECT_EQ("gazebo%40localhost", sanitizeAuthority("gazebo@localhost"));
}

/////////////////////////////////////////////////
TEST(HelpersTEST, UriToPathNoAuthority)
{
#ifdef WIN32
  const std::string testStr0 = R"(localhost:8000)";
  const std::string testStr1 = R"(localhost:8000\some\path)";
  const std::string testStr2 = R"(localhost:8000\some\path\)";
#else
  const std::string testStr0 = R"(localhost:8000)";
  const std::string testStr1 = R"(localhost:8000/some/path)";
  const std::string testStr2 = R"(localhost:8000/some/path/)";
#endif

  {
    common::URI uri{"http://localhost:8000"};
    EXPECT_EQ(testStr0, uriToPath(uri));
  }

  {
    common::URI uri{"http://localhost:8000/some/path"};
    EXPECT_EQ(testStr1, uriToPath(uri));
  }

  {
    common::URI uri{"http://localhost:8000/some/path/"};
    EXPECT_EQ(testStr2, uriToPath(uri));
  }
}

/////////////////////////////////////////////////
TEST(HelpersTEST, UriToPathHasAuthority)
{
#ifdef WIN32
  const std::string testStr0 = R"(localhost%3A8000)";
  const std::string testStr1 = R"(localhost%3A8000\some\path)";
  const std::string testStr2 = R"(localhost%3A8000\some\path\)";
#else
  const std::string testStr0 = R"(localhost%3A8000)";
  const std::string testStr1 = R"(localhost%3A8000/some/path)";
  const std::string testStr2 = R"(localhost%3A8000/some/path/)";
#endif

  {
    common::URI uri{"http://localhost:8000", true};
    EXPECT_EQ(testStr0, uriToPath(uri));
  }

  {
    common::URI uri{"http://localhost:8000/some/path", true};
    EXPECT_EQ(testStr1, uriToPath(uri));
  }

  {
    common::URI uri{"http://localhost:8000/some/path/", true};
    EXPECT_EQ(testStr2, uriToPath(uri));
  }
}
