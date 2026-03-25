/*
 * Copyright (C) 2018 Open Source Robotics Foundation
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

#include "cmd/LogCommandAPI.hh"
#include "gtest/gtest.h"

//////////////////////////////////////////////////
TEST(LogCommandAPI, Version)
{
  EXPECT_EQ(INVALID_VERSION, verbosity(-1));
  EXPECT_EQ(SUCCESS, verbosity(0));
  EXPECT_EQ(INVALID_VERSION, verbosity(5));
}

//////////////////////////////////////////////////
TEST(LogCommandAPI, RecordBadRegex)
{
  EXPECT_EQ(BAD_REGEX, recordTopics(":memory:", "*"));
}

//////////////////////////////////////////////////
TEST(LogCommandAPI, PlaybackBadRegex)
{
  EXPECT_EQ(BAD_REGEX, playbackTopics(":memory:", "*", 0, "", true));
}

//////////////////////////////////////////////////
TEST(LogCommandAPI, PlaybackBadRemap)
{
  EXPECT_EQ(INVALID_REMAP, playbackTopics(":memory:", ".*", 0, "/foo", true));
  EXPECT_EQ(INVALID_REMAP, playbackTopics(":memory:", ".*", 0, "/foo:=",
        false));
  EXPECT_EQ(INVALID_REMAP, playbackTopics(":memory:", ".*", 0, "/foo:= ",
        true));
  EXPECT_EQ(INVALID_REMAP, playbackTopics(":memory:", ".*", 0, ":=/bar",
        false));
  EXPECT_EQ(INVALID_REMAP, playbackTopics(":memory:", ".*", 0, " :=/bar",
        true));
}

//////////////////////////////////////////////////
TEST(LogCommandAPI, RecordFailedToOpen)
{
  EXPECT_EQ(FAILED_TO_OPEN, recordTopics("!@#$%^&*(:;[{]})?/.'|", ".*"));
}

//////////////////////////////////////////////////
TEST(LogCommandAPI, PlaybackFailedToOpen)
{
  EXPECT_EQ(FAILED_TO_OPEN,
    playbackTopics("!@#$%^&*(:;[{]})?/.'|", ".*", 0, "", false));
}
