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

#include <gz/math/Vector3.hh>
#include <gz/math/Quaternion.hh>
#include <gz/common/KeyFrame.hh>
#include <gz/common/Animation.hh>

#include "gz/common/testing/AutoLogFixture.hh"

using namespace gz;

class AnimationTest : public common::testing::AutoLogFixture { };

/////////////////////////////////////////////////
TEST_F(AnimationTest, PoseAnimation)
{
  {
    common::PoseAnimation anim("test", 1.0, true);
    anim.Time(-0.5);
    EXPECT_DOUBLE_EQ(0.5, anim.Time());
  }

  {
    common::PoseAnimation anim("test", 1.0, false);
    anim.Time(-0.5);
    EXPECT_DOUBLE_EQ(0.0, anim.Time());

    anim.Time(1.5);
    EXPECT_DOUBLE_EQ(1.0, anim.Time());
  }

  {
    common::PoseAnimation anim("interpolatex_test", 1.0, true);
    EXPECT_FALSE(anim.InterpolateX());
    anim.SetInterpolateX(true);
    EXPECT_TRUE(anim.InterpolateX());
  }

  common::PoseAnimation anim("pose_test", 5.0, false);
  common::PoseKeyFrame *key = anim.CreateKeyFrame(0.0);

  EXPECT_DOUBLE_EQ(5.0, anim.Length());
  anim.Length(10.0);
  EXPECT_DOUBLE_EQ(10.0, anim.Length());

  key->Translation(math::Vector3d(0, 0, 0));
  EXPECT_TRUE(key->Translation() == math::Vector3d(0, 0, 0));

  key->Rotation(math::Quaterniond(0, 0, 0));
  EXPECT_TRUE(key->Rotation() == math::Quaterniond(0, 0, 0));

  key = anim.CreateKeyFrame(10.0);
  key->Translation(math::Vector3d(10, 20, 30));
  EXPECT_TRUE(key->Translation() == math::Vector3d(10, 20, 30));

  key->Rotation(math::Quaterniond(0.1, 0.2, 0.3));
  EXPECT_TRUE(key->Rotation() == math::Quaterniond(0.1, 0.2, 0.3));

  anim.AddTime(5.0);
  EXPECT_DOUBLE_EQ(5.0, anim.Time());

  anim.Time(4.0);
  EXPECT_DOUBLE_EQ(4.0, anim.Time());

  common::PoseKeyFrame interpolatedKey(-1.0);
  anim.InterpolatedKeyFrame(interpolatedKey);
  EXPECT_TRUE(interpolatedKey.Translation() ==
      math::Vector3d(3.76, 7.52, 11.28));
  EXPECT_TRUE(interpolatedKey.Rotation() ==
      math::Quaterniond(0.0302776, 0.0785971, 0.109824));
}

/////////////////////////////////////////////////
TEST_F(AnimationTest, PoseAnimationTension)
{
  // Test using a tension value of 1.0, which should cause the pose
  // animation to hit the key frames.

  common::PoseAnimation animTension("test-tension", 10.0, false, 1.0);
  common::PoseKeyFrame *keyTension = animTension.CreateKeyFrame(0.0);
  keyTension->Translation(math::Vector3d(0, 0, 0));
  keyTension->Rotation(math::Quaterniond(0, 0, 0));

  animTension.AddTime(5.0);
  keyTension->Translation(math::Vector3d(10, 20, 30));
  keyTension->Rotation(math::Quaterniond(0.1, 0.2, 0.3));
  animTension.Time(4.0);

  common::PoseKeyFrame interpolatedKeyTension(-1.0);
  animTension.InterpolatedKeyFrame(interpolatedKeyTension);
  EXPECT_TRUE(interpolatedKeyTension.Translation() ==
      math::Vector3d(10, 20, 30));
  EXPECT_TRUE(interpolatedKeyTension.Rotation() ==
      math::Quaterniond(0.1, 0.2, 0.3));
}

/////////////////////////////////////////////////
TEST_F(AnimationTest, NumericAnimation)
{
  common::NumericAnimation anim("numeric_test", 10, false);
  common::NumericKeyFrame *key = anim.CreateKeyFrame(0.0);

  key->Value(0.0);
  EXPECT_DOUBLE_EQ(0.0, key->Value());

  key = anim.CreateKeyFrame(10.0);
  key->Value(30);
  EXPECT_DOUBLE_EQ(30, key->Value());

  anim.AddTime(5.0);
  EXPECT_DOUBLE_EQ(5.0, anim.Time());

  anim.Time(4.0);
  EXPECT_DOUBLE_EQ(4.0, anim.Time());

  common::NumericKeyFrame interpolatedKey(0);
  anim.InterpolatedKeyFrame(interpolatedKey);
  EXPECT_DOUBLE_EQ(12, interpolatedKey.Value());
}

/////////////////////////////////////////////////
TEST_F(AnimationTest, TrajectoryInfo)
{
  using namespace std::chrono_literals;
  using TP = std::chrono::steady_clock::time_point;

  common::TrajectoryInfo trajInfo;

  trajInfo.SetId(0);
  EXPECT_EQ(0u, trajInfo.Id());

  trajInfo.SetAnimIndex(0);
  EXPECT_EQ(0u, trajInfo.AnimIndex());

  auto startTime = TP(3s);
  trajInfo.SetStartTime(startTime);
  EXPECT_EQ(startTime, trajInfo.StartTime());

  auto endTime = TP(5s);
  trajInfo.SetEndTime(endTime);
  EXPECT_EQ(endTime, trajInfo.EndTime());

  // Duration is calculated from start and end times
  EXPECT_EQ(2s, trajInfo.Duration());

  trajInfo.SetTranslated(true);
  EXPECT_TRUE(trajInfo.Translated());

  EXPECT_EQ(nullptr, trajInfo.Waypoints());

  std::map<TP, math::Pose3d> waypoints;
  // duration from start == 0
  waypoints[TP(100ms)] = math::Pose3d::Zero;
  // duration from start == 100ms
  waypoints[TP(200ms)] = math::Pose3d(1, 0, 0, 0, 0, 0);
  // duration from start == 150ms
  waypoints[TP(250ms)] = math::Pose3d(2, 0, 0, 0, 0, 0);
  // duration from start == 200ms
  waypoints[TP(300ms)] = math::Pose3d(4, 0, 0, 0, 0, 0);

  trajInfo.SetWaypoints(waypoints);
  EXPECT_NE(nullptr, trajInfo.Waypoints());

  // Start and end times updated from new waypoints
  EXPECT_EQ(TP(100ms), trajInfo.StartTime());
  EXPECT_EQ(TP(300ms), trajInfo.EndTime());
  EXPECT_EQ(200ms, trajInfo.Duration());

  EXPECT_DOUBLE_EQ(0.0, trajInfo.DistanceSoFar(0ms));
  EXPECT_DOUBLE_EQ(0.5, trajInfo.DistanceSoFar(50ms));
  EXPECT_DOUBLE_EQ(1.0, trajInfo.DistanceSoFar(100ms));
  EXPECT_DOUBLE_EQ(2.0, trajInfo.DistanceSoFar(150ms));
  EXPECT_DOUBLE_EQ(3.0, trajInfo.DistanceSoFar(175ms));
  EXPECT_DOUBLE_EQ(4.0, trajInfo.DistanceSoFar(200ms));
  EXPECT_DOUBLE_EQ(4.0, trajInfo.DistanceSoFar(500ms));

  waypoints.clear();
  // duration from start == 0
  waypoints[TP(200ms)] = math::Pose3d(1, 0, 0, 0, 0, 0);
  // duration from start == 2000ms
  waypoints[TP(2200ms)] = math::Pose3d(1, 0, 0, 0, 0, 0);

  trajInfo.SetWaypoints(waypoints);
  EXPECT_NE(nullptr, trajInfo.Waypoints());

  // Start and end times updated from new waypoints
  EXPECT_EQ(TP(200ms), trajInfo.StartTime());
  EXPECT_EQ(TP(2200ms), trajInfo.EndTime());
  EXPECT_EQ(2000ms, trajInfo.Duration());

  EXPECT_DOUBLE_EQ(0.0, trajInfo.DistanceSoFar(0ms));
  EXPECT_DOUBLE_EQ(0.0, trajInfo.DistanceSoFar(100ms));
  EXPECT_DOUBLE_EQ(0.0, trajInfo.DistanceSoFar(200ms));
  EXPECT_DOUBLE_EQ(0.0, trajInfo.DistanceSoFar(300ms));
  EXPECT_DOUBLE_EQ(0.0, trajInfo.DistanceSoFar(500ms));
  EXPECT_DOUBLE_EQ(0.0, trajInfo.DistanceSoFar(1000ms));
  EXPECT_DOUBLE_EQ(0.0, trajInfo.DistanceSoFar(2000ms));
  EXPECT_DOUBLE_EQ(0.0, trajInfo.DistanceSoFar(3000ms));

  // copy construstor
  common::TrajectoryInfo trajInfo2(trajInfo);
  EXPECT_EQ(0u, trajInfo2.Id());
  EXPECT_EQ(0u, trajInfo2.AnimIndex());
  EXPECT_EQ(2s, trajInfo2.Duration());
  EXPECT_TRUE(trajInfo2.Translated());
  EXPECT_NE(nullptr, trajInfo2.Waypoints());
  EXPECT_EQ(TP(200ms), trajInfo2.StartTime());
  EXPECT_EQ(TP(2200ms), trajInfo2.EndTime());
  EXPECT_EQ(2000ms, trajInfo2.Duration());
  EXPECT_DOUBLE_EQ(0.0, trajInfo2.DistanceSoFar(0ms));
  EXPECT_DOUBLE_EQ(0.0, trajInfo2.DistanceSoFar(100ms));
  EXPECT_DOUBLE_EQ(0.0, trajInfo2.DistanceSoFar(200ms));
  EXPECT_DOUBLE_EQ(0.0, trajInfo2.DistanceSoFar(300ms));
  EXPECT_DOUBLE_EQ(0.0, trajInfo2.DistanceSoFar(500ms));
  EXPECT_DOUBLE_EQ(0.0, trajInfo2.DistanceSoFar(1000ms));
  EXPECT_DOUBLE_EQ(0.0, trajInfo2.DistanceSoFar(2000ms));
  EXPECT_DOUBLE_EQ(0.0, trajInfo2.DistanceSoFar(3000ms));

  // copy assignment operator
  common::TrajectoryInfo trajInfo3;
  trajInfo3 = trajInfo;
  EXPECT_EQ(0u, trajInfo3.Id());
  EXPECT_EQ(0u, trajInfo3.AnimIndex());
  EXPECT_EQ(2s, trajInfo3.Duration());
  EXPECT_TRUE(trajInfo3.Translated());
  EXPECT_NE(nullptr, trajInfo3.Waypoints());
  EXPECT_EQ(TP(200ms), trajInfo3.StartTime());
  EXPECT_EQ(TP(2200ms), trajInfo3.EndTime());
  EXPECT_EQ(2000ms, trajInfo3.Duration());
  EXPECT_DOUBLE_EQ(0.0, trajInfo3.DistanceSoFar(0ms));
  EXPECT_DOUBLE_EQ(0.0, trajInfo3.DistanceSoFar(100ms));
  EXPECT_DOUBLE_EQ(0.0, trajInfo3.DistanceSoFar(200ms));
  EXPECT_DOUBLE_EQ(0.0, trajInfo3.DistanceSoFar(300ms));
  EXPECT_DOUBLE_EQ(0.0, trajInfo3.DistanceSoFar(500ms));
  EXPECT_DOUBLE_EQ(0.0, trajInfo3.DistanceSoFar(1000ms));
  EXPECT_DOUBLE_EQ(0.0, trajInfo3.DistanceSoFar(2000ms));
  EXPECT_DOUBLE_EQ(0.0, trajInfo3.DistanceSoFar(3000ms));

  // move constructor
  common::TrajectoryInfo trajInfo4(std::move(trajInfo));
  EXPECT_EQ(0u, trajInfo4.Id());
  EXPECT_EQ(0u, trajInfo4.AnimIndex());
  EXPECT_EQ(2s, trajInfo4.Duration());
  EXPECT_TRUE(trajInfo4.Translated());
  EXPECT_NE(nullptr, trajInfo4.Waypoints());
  EXPECT_EQ(TP(200ms), trajInfo4.StartTime());
  EXPECT_EQ(TP(2200ms), trajInfo4.EndTime());
  EXPECT_EQ(2000ms, trajInfo4.Duration());
  EXPECT_DOUBLE_EQ(0.0, trajInfo4.DistanceSoFar(0ms));
  EXPECT_DOUBLE_EQ(0.0, trajInfo4.DistanceSoFar(100ms));
  EXPECT_DOUBLE_EQ(0.0, trajInfo4.DistanceSoFar(200ms));
  EXPECT_DOUBLE_EQ(0.0, trajInfo4.DistanceSoFar(300ms));
  EXPECT_DOUBLE_EQ(0.0, trajInfo4.DistanceSoFar(500ms));
  EXPECT_DOUBLE_EQ(0.0, trajInfo4.DistanceSoFar(1000ms));
  EXPECT_DOUBLE_EQ(0.0, trajInfo4.DistanceSoFar(2000ms));
  EXPECT_DOUBLE_EQ(0.0, trajInfo4.DistanceSoFar(3000ms));
}
