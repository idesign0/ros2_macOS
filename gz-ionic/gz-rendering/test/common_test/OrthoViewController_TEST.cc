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

#include "CommonRenderingTest.hh"

#include "gz/rendering/Camera.hh"
#include "gz/rendering/OrthoViewController.hh"
#include "gz/rendering/Scene.hh"

using namespace gz;
using namespace rendering;

class OrthoViewControllerTest : public CommonRenderingTest 
{
};

/////////////////////////////////////////////////
TEST_F(OrthoViewControllerTest, OrthoViewControl)
{
  ScenePtr scene = engine->CreateScene("scene");
  EXPECT_NE(nullptr, scene);

  CameraPtr camera =  scene->CreateCamera("camera");
  EXPECT_NE(nullptr, camera);

  OrthoViewController viewControl;

  // verify intial values
  EXPECT_EQ(nullptr, viewControl.Camera());
  EXPECT_EQ(math::Vector3d::Zero, viewControl.Target());

  // test setting caream
  viewControl.SetCamera(camera);
  EXPECT_EQ(camera, viewControl.Camera());

  viewControl.SetCamera(CameraPtr());
  EXPECT_EQ(CameraPtr(), viewControl.Camera());

  // test setting target
  math::Vector3d target(1, 0, 0);
  viewControl.SetTarget(target);
  EXPECT_EQ(target, viewControl.Target());

  target.Set(-0.3, -5, 1);
  viewControl.SetTarget(target);
  EXPECT_EQ(target, viewControl.Target());

  // Clean up
  engine->DestroyScene(scene);
}

/////////////////////////////////////////////////
TEST_F(OrthoViewControllerTest, Control)
{
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  CameraPtr camera =  scene->CreateCamera("camera");
  ASSERT_NE(nullptr, camera);

  camera->SetImageWidth(320);
  camera->SetImageHeight(240);

  math::Pose3d initialPose = camera->WorldPose();
  EXPECT_EQ(math::Pose3d::Zero, initialPose);

  OrthoViewController viewControl;

  // test setting camera
  viewControl.SetCamera(camera);
  EXPECT_EQ(camera, viewControl.Camera());

  // test setting target
  math::Vector3d target(1, 0, 0);
  viewControl.SetTarget(target);
  EXPECT_EQ(target, viewControl.Target());

  // test zoom
  math::Matrix4d initialProjectionMatrix = camera->ProjectionMatrix();
  viewControl.Zoom(0);
  EXPECT_EQ(math::Pose3d::Zero, initialPose);
  viewControl.Zoom(0.1);
  // we do not change the camera pos when zooming in orthographic view
  // we update the projection matrix
  EXPECT_EQ(initialPose.Pos(), camera->WorldPose().Pos());
  EXPECT_EQ(initialPose.Rot(), camera->WorldPose().Rot());
  math::Matrix4d zoomProjectionMatrix = camera->ProjectionMatrix();
  EXPECT_NE(initialProjectionMatrix, zoomProjectionMatrix);
  viewControl.Zoom(-5);
  math::Matrix4d zoomProjectionMatrix2 = camera->ProjectionMatrix();
  EXPECT_EQ(initialPose.Pos(), camera->WorldPose().Pos());
  EXPECT_EQ(initialPose.Rot(), camera->WorldPose().Rot());
  EXPECT_NE(initialProjectionMatrix, zoomProjectionMatrix2);
  EXPECT_NE(zoomProjectionMatrix, zoomProjectionMatrix2);

  // reset camera pose
  camera->SetWorldPose(initialPose);
  EXPECT_EQ(math::Pose3d::Zero, initialPose);

  // test zero pan
  viewControl.Pan(math::Vector2d(0, 0));
  EXPECT_EQ(math::Pose3d::Zero, initialPose);

  // test pan in viewport x
  viewControl.Pan(math::Vector2d(2, 0));
  EXPECT_DOUBLE_EQ(initialPose.Pos().X(), camera->WorldPose().Pos().X());
  EXPECT_LT(initialPose.Pos().Y(), camera->WorldPose().Pos().Y());
  EXPECT_DOUBLE_EQ(initialPose.Pos().Z(), camera->WorldPose().Pos().Z());
  EXPECT_EQ(initialPose.Rot(), camera->WorldPose().Rot());
  viewControl.Pan(math::Vector2d(-8, 0));
  EXPECT_DOUBLE_EQ(initialPose.Pos().X(), camera->WorldPose().Pos().X());
  EXPECT_GT(initialPose.Pos().Y(), camera->WorldPose().Pos().Y());
  EXPECT_DOUBLE_EQ(initialPose.Pos().Z(), camera->WorldPose().Pos().Z());
  EXPECT_EQ(initialPose.Rot(), camera->WorldPose().Rot());

  // reset camera pose
  camera->SetWorldPose(initialPose);
  EXPECT_EQ(math::Pose3d::Zero, initialPose);

  // test pan in viewport y
  viewControl.Pan(math::Vector2d(0, 7));
  EXPECT_DOUBLE_EQ(initialPose.Pos().X(), camera->WorldPose().Pos().X());
  EXPECT_DOUBLE_EQ(initialPose.Pos().Y(), camera->WorldPose().Pos().Y());
  EXPECT_LT(initialPose.Pos().Z(), camera->WorldPose().Pos().Z());
  EXPECT_EQ(initialPose.Rot(), camera->WorldPose().Rot());
  viewControl.Pan(math::Vector2d(0, -25));
  EXPECT_DOUBLE_EQ(initialPose.Pos().X(), camera->WorldPose().Pos().X());
  EXPECT_DOUBLE_EQ(initialPose.Pos().Y(), camera->WorldPose().Pos().Y());
  EXPECT_GT(initialPose.Pos().Z(), camera->WorldPose().Pos().Z());
  EXPECT_EQ(initialPose.Rot(), camera->WorldPose().Rot());

  // reset camera pose
  camera->SetWorldPose(initialPose);
  EXPECT_EQ(math::Pose3d::Zero, initialPose);

  // test zero orbit
  viewControl.Orbit(math::Vector2d(0, 0));
  EXPECT_EQ(math::Pose3d::Zero, initialPose);

  // test orbit in viewport x (yaw)
  viewControl.Orbit(math::Vector2d(100, 0));
  EXPECT_LT(initialPose.Pos().X(), camera->WorldPose().Pos().X());
  EXPECT_LT(initialPose.Pos().Y(), camera->WorldPose().Pos().Y());
  EXPECT_DOUBLE_EQ(initialPose.Pos().Z(), camera->WorldPose().Pos().Z());
  EXPECT_DOUBLE_EQ(initialPose.Rot().X(), camera->WorldPose().Rot().X());
  EXPECT_DOUBLE_EQ(initialPose.Rot().Y(), camera->WorldPose().Rot().Y());
  EXPECT_GT(initialPose.Rot().Z(), camera->WorldPose().Rot().Z());

  camera->SetWorldPose(initialPose);
  EXPECT_EQ(math::Pose3d::Zero, initialPose);

  viewControl.Orbit(math::Vector2d(-80, 0));
  EXPECT_LT(initialPose.Pos().X(), camera->WorldPose().Pos().X());
  EXPECT_GT(initialPose.Pos().Y(), camera->WorldPose().Pos().Y());
  EXPECT_DOUBLE_EQ(initialPose.Pos().Z(), camera->WorldPose().Pos().Z());
  EXPECT_DOUBLE_EQ(initialPose.Rot().X(), camera->WorldPose().Rot().X());
  EXPECT_DOUBLE_EQ(initialPose.Rot().Y(), camera->WorldPose().Rot().Y());
  EXPECT_LT(initialPose.Rot().Z(), camera->WorldPose().Rot().Z());

  // reset camera pose
  camera->SetWorldPose(initialPose);
  EXPECT_EQ(math::Pose3d::Zero, initialPose);

  // test orbit in viewport y (pitch)
  viewControl.Orbit(math::Vector2d(0, 80));
  EXPECT_LT(initialPose.Pos().X(), camera->WorldPose().Pos().X());
  EXPECT_DOUBLE_EQ(initialPose.Pos().Y(), camera->WorldPose().Pos().Y());
  EXPECT_LT(initialPose.Pos().Z(), camera->WorldPose().Pos().Z());
  EXPECT_DOUBLE_EQ(initialPose.Rot().X(), camera->WorldPose().Rot().X());
  EXPECT_LT(initialPose.Rot().Y(), camera->WorldPose().Rot().Y());
  EXPECT_DOUBLE_EQ(initialPose.Rot().Z(), camera->WorldPose().Rot().Z());

  camera->SetWorldPose(initialPose);
  EXPECT_EQ(math::Pose3d::Zero, initialPose);

  viewControl.Orbit(math::Vector2d(0, -90));
  EXPECT_LT(initialPose.Pos().X(), camera->WorldPose().Pos().X());
  EXPECT_DOUBLE_EQ(initialPose.Pos().Y(), camera->WorldPose().Pos().Y());
  EXPECT_GT(initialPose.Pos().Z(), camera->WorldPose().Pos().Z());
  EXPECT_DOUBLE_EQ(initialPose.Rot().X(), camera->WorldPose().Rot().X());
  EXPECT_GT(initialPose.Rot().Y(), camera->WorldPose().Rot().Y());
  EXPECT_DOUBLE_EQ(initialPose.Rot().Z(), camera->WorldPose().Rot().Z());

  // Clean up
  engine->DestroyScene(scene);
}
