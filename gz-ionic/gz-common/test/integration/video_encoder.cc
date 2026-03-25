/*
 * Copyright (C) 2020 Open Source Robotics Foundation
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

// needed on MacOS
#include <cmath>

#include "gz/common/Console.hh"
#include "gz/common/VideoEncoder.hh"
#include "gz/common/Video.hh"
#include "gz/common/ffmpeg_inc.hh"

#include <gz/common/testing/AutoLogFixture.hh>
#include <gz/common/testing/TestPaths.hh>

using namespace gz;
using namespace common;
using namespace std::chrono;
auto Now = steady_clock::now;
using TimePoint = steady_clock::time_point;

class EncoderDecoderTest : public common::testing::AutoLogFixture { };

double
computeAverageIntensity(const size_t bufferSize, const unsigned char* buf)
{
  double avgIntensity = 0.0;
  for (size_t i = 0; i < bufferSize; ++i)
  {
    avgIntensity += static_cast<double>(buf[i]);
  }
  avgIntensity = avgIntensity / bufferSize;
  return avgIntensity;
}

/////////////////////////////////////////////////
TEST_F(EncoderDecoderTest, DecodeEncodeDecode)
{
  av_log_set_level(AV_LOG_ERROR);

  const unsigned int fps = 25u;

  const char* testVideoInName = "test_video.mp4";
  const char* testVideoOutName = "test_video_out.mp4";

  const auto testVideoInPath =
    common::testing::TestFile("data", testVideoInName);
  const auto testVideoOutPath = common::testing::TempPath(testVideoOutName);

  Video decoder;
  decoder.Load(testVideoInPath);

  VideoEncoder encoder;
  ASSERT_TRUE(encoder.Start("mp4", "",
                            decoder.Width(), decoder.Height(), fps));

  const size_t bufferSize = decoder.Width() * decoder.Height() * 3;
  auto* buf = new unsigned char[bufferSize];
  size_t numInFrames = 0;
  size_t numOutFrames = 0;
  double avgIntensity = -1.0;

  auto fps_start = Now();
  size_t fps_frames = 0;

  while (decoder.NextFrame(&buf))
  {
    TimePoint stamp { milliseconds(1000) / fps * numInFrames };
    numInFrames++;

    if (encoder.AddFrame(buf, decoder.Width(), decoder.Height(), stamp))
      numOutFrames++;

    // compute average intensity of frame number 10
    if (numInFrames == 10)
      avgIntensity = computeAverageIntensity(bufferSize, buf);

    fps_frames++;
    if ((Now() - fps_start) > seconds(1)) {
      double dur = duration<double>(Now() - fps_start).count();
      double currentFps = fps_frames / dur;
      gzmsg << "FPS: " << currentFps << std::endl;
      fps_start = Now();
      fps_frames = 0;
    }
  }

  EXPECT_EQ(numInFrames, 90u);
  EXPECT_EQ(numOutFrames, 90u);

  // check that the intensity got computed and that the image is not all black
  // or all white
  ASSERT_GT(std::abs(avgIntensity - (-1.0)), 1e-6);
  EXPECT_GT(avgIntensity, 1e-6);
  EXPECT_LT(avgIntensity, 255.0 - 1e-6);

  encoder.SaveToFile(testVideoOutPath);
  ASSERT_TRUE(exists(testVideoOutPath));

  Video decoder2;
  decoder2.Load(testVideoOutPath);

  ASSERT_EQ(decoder2.Width(), decoder.Width());
  ASSERT_EQ(decoder2.Height(), decoder.Height());

  size_t numFrames2 = 0;
  double avgIntensity2 = -1.0;
  while (decoder2.NextFrame(&buf))
  {
    numFrames2++;
    if (numFrames2 == 10)
      avgIntensity2 = computeAverageIntensity(bufferSize, buf);
  }

  // one frame is still being lost somewhere, but only one regardless of video
  // length or fps
  size_t expectedNumFrames2 =
#if defined(_WIN32)  // Windows on buildfarm sometimes lose 2 frames
     88u;
#else
     89u;
#endif

  EXPECT_GE(numFrames2, expectedNumFrames2);
  // average color intensities should be pretty close
  EXPECT_NEAR(avgIntensity2, avgIntensity, 2.0);

  delete[] buf;
}
