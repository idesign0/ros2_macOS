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
#include <array>
#include "gz/common/VideoEncoder.hh"
#include "gz/common/Video.hh"

using namespace gz;
using namespace common;

const unsigned int kSize = 10;
const std::array<unsigned char, kSize*kSize*3> kFrame = {};

// set to 720ms because video duration missing additional 18 frames
//    which may be due to how video encoding works
const std::chrono::milliseconds kTol(720);

void durationTest(VideoEncoder &_vidEncoder, Video &_video,
                  const int &_fps, const int &_seconds)
{
  _vidEncoder.Start("mp4", "", kSize, kSize, _fps, 0);

  int frameCount = 0;
  while (frameCount != _fps*_seconds)
  {
    if (_vidEncoder.AddFrame(kFrame.data(), kSize, kSize))
      ++frameCount;
  }

  _vidEncoder.Stop();
  _video.Load(common::joinPaths(cwd(), "TMP_RECORDING.mp4"));

  auto length = std::chrono::duration_cast<std::chrono::milliseconds>(
      _video.Duration()).count();

  // Static cast to suppress Windows warning
  EXPECT_NEAR(static_cast<double>(length),
              static_cast<double>(_seconds*1000),
              static_cast<double>(kTol.count()));
}

TEST(EncoderTimingTest, Duration)
{
  VideoEncoder vidEncoder;
  Video video;

  durationTest(vidEncoder, video, 50, 1);
  durationTest(vidEncoder, video, 30, 2);
  durationTest(vidEncoder, video, 25, 5);
}
