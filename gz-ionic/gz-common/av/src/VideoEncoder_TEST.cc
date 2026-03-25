/*
 * Copyright (C) 2016 Open Source Robotics Foundation
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
#include "gz/common/VideoEncoder.hh"

#include "gz/common/testing/AutoLogFixture.hh"
#include "gz/common/testing/TestPaths.hh"

using namespace gz;
using namespace common;

class VideoEncoderTest : public common::testing::AutoLogFixture
{
  // Documentation inherited
  protected: void SetUp() override
  {
    Console::SetVerbosity(4);
    tempDir = common::testing::MakeTestTempDirectory();
    ASSERT_TRUE(tempDir->Valid()) << tempDir->Path();
  }

  public: std::shared_ptr<gz::common::TempDirectory> tempDir;
};

/////////////////////////////////////////////////
TEST_F(VideoEncoderTest, StartStopDefault)
{
  auto filePathMp4 = common::joinPaths(cwd(), "TMP_RECORDING.mp4");

  {
    VideoEncoder video;
    EXPECT_FALSE(video.IsEncoding());
    EXPECT_STREQ(video.Format().c_str(), VIDEO_ENCODER_FORMAT_DEFAULT);
    EXPECT_EQ(video.BitRate(), static_cast<unsigned int>(
          VIDEO_ENCODER_BITRATE_DEFAULT));

    EXPECT_TRUE(video.Start());
    EXPECT_TRUE(video.IsEncoding());
    EXPECT_TRUE(exists(filePathMp4)) << filePathMp4;
    EXPECT_EQ(video.BitRate(), 920000u);

    EXPECT_TRUE(video.Stop());
    EXPECT_FALSE(video.IsEncoding());
  }

  // Check that temp files are removed when video goes out of scope
  EXPECT_FALSE(exists(filePathMp4)) << filePathMp4;
}

/////////////////////////////////////////////////
TEST_F(VideoEncoderTest, StartStopMpg)
{
  auto filePathMpg = common::joinPaths(cwd(), "TMP_RECORDING.mpg");

  {
    VideoEncoder video;
    EXPECT_FALSE(video.IsEncoding());
    EXPECT_STREQ(video.Format().c_str(), VIDEO_ENCODER_FORMAT_DEFAULT);
    EXPECT_EQ(video.BitRate(), static_cast<unsigned int>(
          VIDEO_ENCODER_BITRATE_DEFAULT));

    EXPECT_TRUE(video.Start("mpg", "", 1024, 768));
    EXPECT_TRUE(video.IsEncoding());
    EXPECT_STREQ(video.Format().c_str(), "mpg");
    EXPECT_TRUE(exists(filePathMpg)) << filePathMpg;
    EXPECT_TRUE(video.Stop());
    EXPECT_FALSE(video.IsEncoding());
  }

  EXPECT_FALSE(exists(filePathMpg)) << filePathMpg;
}


/////////////////////////////////////////////////
TEST_F(VideoEncoderTest, StartStopMp4)
{
  auto filePathMp4 = common::joinPaths(cwd(), "TMP_RECORDING.mp4");

  {
    VideoEncoder video;
    EXPECT_FALSE(video.IsEncoding());
    EXPECT_STREQ(video.Format().c_str(), VIDEO_ENCODER_FORMAT_DEFAULT);
    EXPECT_EQ(video.BitRate(), static_cast<unsigned int>(
          VIDEO_ENCODER_BITRATE_DEFAULT));

    EXPECT_TRUE(video.Start("mp4", "", 1024, 768));
    EXPECT_TRUE(video.IsEncoding());
    EXPECT_STREQ(video.Format().c_str(), "mp4");
    EXPECT_TRUE(exists(filePathMp4)) << filePathMp4;
    video.Stop();
    EXPECT_FALSE(video.IsEncoding());
  }

  EXPECT_FALSE(exists(filePathMp4)) << filePathMp4;
}

/////////////////////////////////////////////////
TEST_F(VideoEncoderTest, RepeatedStart)
{
  auto filePathMpg = common::joinPaths(cwd(), "TMP_RECORDING.mpg");
  auto filePathMp4 = common::joinPaths(cwd(), "TMP_RECORDING.mp4");

  {
    VideoEncoder video;
    EXPECT_FALSE(video.IsEncoding());
    EXPECT_STREQ(video.Format().c_str(), VIDEO_ENCODER_FORMAT_DEFAULT);
    EXPECT_EQ(video.BitRate(), static_cast<unsigned int>(
          VIDEO_ENCODER_BITRATE_DEFAULT));

    EXPECT_TRUE(video.Start("mp4", "", 1024, 768));
    EXPECT_TRUE(video.IsEncoding());
    EXPECT_STREQ(video.Format().c_str(), "mp4");
    EXPECT_TRUE(exists(filePathMp4)) << filePathMp4;

    // Calling start again should return false and not mutate any
    // internal state of the VideoEncoder
    EXPECT_FALSE(video.Start("mpg", "", 1024, 768));
    EXPECT_TRUE(video.IsEncoding());
    EXPECT_STREQ(video.Format().c_str(), "mp4");
    EXPECT_TRUE(exists(filePathMp4)) << filePathMp4;
    EXPECT_FALSE(exists(filePathMpg)) << filePathMpg;

    EXPECT_TRUE(video.Stop());
    EXPECT_FALSE(video.IsEncoding());

    // Once the VideoEncoder has been stopped, a new run may start.
    EXPECT_TRUE(video.Start("mpg", "", 1024, 768));
    EXPECT_TRUE(video.IsEncoding());
    EXPECT_STREQ(video.Format().c_str(), "mpg");
    EXPECT_FALSE(exists(filePathMp4)) << filePathMp4;
    EXPECT_TRUE(exists(filePathMpg)) << filePathMpg;
  }

  // All temporary files will be removed after exiting scope.
  EXPECT_FALSE(exists(filePathMp4)) << filePathMp4;
  EXPECT_FALSE(exists(filePathMpg)) << filePathMp4;
}


/////////////////////////////////////////////////
TEST_F(VideoEncoderTest, Exists)
{
  auto filePathMp4 = common::joinPaths(cwd(), "TMP_RECORDING.mp4");
  auto filePathMpg = common::joinPaths(cwd(), "TMP_RECORDING.mpg");

  {
    VideoEncoder video;
    EXPECT_FALSE(video.IsEncoding());
    EXPECT_STREQ(video.Format().c_str(), VIDEO_ENCODER_FORMAT_DEFAULT);

    EXPECT_FALSE(exists(filePathMp4)) << filePathMp4;
    EXPECT_FALSE(exists(filePathMpg)) << filePathMpg;

    video.Start();
    EXPECT_TRUE(exists(filePathMp4));

    video.Reset();
    EXPECT_FALSE(exists(filePathMp4));
  }

  // Check that temp files are removed when video goes out of scope
  EXPECT_FALSE(exists(filePathMp4)) << filePathMp4;
  EXPECT_FALSE(exists(filePathMpg)) << filePathMpg;
}
