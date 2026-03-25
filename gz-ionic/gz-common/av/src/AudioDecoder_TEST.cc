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

#include <gz/common/AudioDecoder.hh>
#include <gz/utils/ExtraTestMacros.hh>
#include <gz/common/testing/TestPaths.hh>

using namespace gz;

/////////////////////////////////////////////////
TEST(AudioDecoder, FileNotSet)
{
  common::AudioDecoder audio;
  unsigned int dataBufferSize;
  uint8_t *dataBuffer = NULL;
  EXPECT_FALSE(audio.Decode(&dataBuffer, &dataBufferSize));

  EXPECT_EQ(audio.File(), "");
  EXPECT_EQ(audio.SampleRate(), -1);
}

/////////////////////////////////////////////////
TEST(AudioDecoder, MissingFile)
{
  common::AudioDecoder audio;
  unsigned int dataBufferSize;
  uint8_t *dataBuffer = NULL;
  EXPECT_FALSE(audio.Decode(&dataBuffer, &dataBufferSize));
}

/////////////////////////////////////////////////
TEST(AudioDecoder, BufferSizeInvalid)
{
  common::AudioDecoder audio;
  auto path = common::testing::TestFile("data", "cheer.wav");
  EXPECT_TRUE(audio.SetFile(path));

  unsigned int *dataBufferSize = NULL;
  uint8_t *dataBuffer = NULL;
  EXPECT_FALSE(audio.Decode(&dataBuffer, dataBufferSize));
}

/////////////////////////////////////////////////
TEST(AudioDecoder, DataBuffer)
{
  common::AudioDecoder audio;

  auto path = common::testing::TestFile("data", "cheer.wav");
  EXPECT_TRUE(audio.SetFile(path));

  unsigned int dataBufferSize;
  uint8_t *dataBuffer = NULL;
  EXPECT_TRUE(audio.Decode(&dataBuffer, &dataBufferSize));

  unsigned int dataBufferSize2;
  uint8_t *dataBuffer2 = static_cast<uint8_t *>(malloc(5));
  EXPECT_TRUE(audio.Decode(&dataBuffer2, &dataBufferSize2));

  EXPECT_EQ(dataBufferSize2, dataBufferSize);
  EXPECT_EQ(sizeof(dataBuffer), sizeof(dataBuffer2));
}

/////////////////////////////////////////////////
TEST(AudioDecoder, NoCodec)
{
  common::AudioDecoder audio;
  auto path = common::testing::TestFile("data", "audio_bad_codec.mp4");
  EXPECT_FALSE(audio.SetFile(path));
}

/////////////////////////////////////////////////
TEST(AudioDecoder, GZ_UTILS_TEST_DISABLED_ON_WIN32(CheerFile))
{
  common::AudioDecoder audio;

  // Test a bad filename
  EXPECT_FALSE(audio.SetFile("_bad_audio_filename_.wav"));

  // Test no stream info
  auto path = common::testing::TestFile("data", "audio_bad_codec.grf");
  EXPECT_FALSE(common::exists(path));
  EXPECT_FALSE(audio.SetFile(path));

  // Test a valid file without an audio stream
  path = common::testing::TestFile("data", "empty_audio.mp4");
  ASSERT_TRUE(common::exists(path));
  EXPECT_FALSE(audio.SetFile(path));

  unsigned int dataBufferSize;
  uint8_t *dataBuffer = nullptr;

  // WAV
  {
    path = common::testing::TestFile("data", "cheer.wav");
    ASSERT_TRUE(common::exists(path));
    EXPECT_TRUE(audio.SetFile(path));
    EXPECT_EQ(audio.File(), path);
    EXPECT_EQ(audio.SampleRate(), 48000);

    audio.Decode(&dataBuffer, &dataBufferSize);
    EXPECT_EQ(dataBufferSize, 5428692u);
  }

  // OGG
  {
    path = common::testing::TestFile("data", "cheer.ogg");
    ASSERT_TRUE(common::exists(path));
    EXPECT_TRUE(audio.SetFile(path));
    EXPECT_EQ(audio.File(), path);
    EXPECT_EQ(audio.SampleRate(), 44100);

    audio.Decode(&dataBuffer, &dataBufferSize);
    // In Ubuntu trusty the buffer size double for ogg decoding.
    // This check is suitable for both older and newer versions of Ubuntu.
    // With ffmpeg 5.0 the value changed again (third value)
    EXPECT_TRUE(dataBufferSize == 4989184u ||
                dataBufferSize == 4989184u * 2u ||
                dataBufferSize == 9975224u);
  }

  // MP3
  {
    path = common::testing::TestFile("data", "cheer.mp3");
    ASSERT_TRUE(common::exists(path));
    EXPECT_TRUE(audio.SetFile(path));
    EXPECT_EQ(audio.File(), path);
    EXPECT_EQ(audio.SampleRate(), 44100);

    audio.Decode(&dataBuffer, &dataBufferSize);

    // later versions of ffmpeg produces a different buffer size probably due to
    // underlying changes in the decoder. The size of the first decoded frame
    // is much smaller than all other frames.
    EXPECT_TRUE(dataBufferSize == 4995072u ||
                dataBufferSize == 4987612u ||
                dataBufferSize == 4987612u * 2);
  }
}
