/*
 * Copyright (C) 2019 Open Source Robotics Foundation
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
#include "gz/rendering/Scene.hh"
#include "gz/rendering/TransformController.hh"

using namespace gz;
using namespace rendering;

class TransformControllerTest : public CommonRenderingTest 
{
};

/////////////////////////////////////////////////
TEST_F(TransformControllerTest, TransformControl)
{
  ScenePtr scene = engine->CreateScene("scene");
  EXPECT_NE(scene, nullptr);

  CameraPtr camera =  scene->CreateCamera("camera");
  EXPECT_NE(camera, nullptr);

  TransformController transformControl;

  // verify intial values
  EXPECT_EQ(nullptr, transformControl.Camera());
  EXPECT_EQ(nullptr, transformControl.Node());
  EXPECT_FALSE(transformControl.Active());
  EXPECT_EQ(TransformMode::TM_NONE, transformControl.Mode());
  EXPECT_EQ(TransformSpace::TS_LOCAL, transformControl.Space());
  EXPECT_EQ(math::Vector3d::Zero, transformControl.ActiveAxis());

  // create visual node for testing
  VisualPtr visual = scene->CreateVisual();
  ASSERT_NE(nullptr, visual);

  // test attaching / detaching node
  transformControl.Attach(visual);
  EXPECT_EQ(visual, transformControl.Node());

  transformControl.Detach();
  EXPECT_EQ(nullptr, transformControl.Node());

  // attach node again for subsequent tests
  transformControl.Attach(visual);

  // test setting camera
  transformControl.SetCamera(camera);
  EXPECT_EQ(camera, transformControl.Camera());

  // test setting transform space
  transformControl.SetTransformSpace(TransformSpace::TS_WORLD);
  EXPECT_EQ(TransformSpace::TS_WORLD, transformControl.Space());

  // test setting transform axis
  transformControl.SetActiveAxis(math::Vector3d::UnitZ);
  EXPECT_EQ(math::Vector3d::UnitZ, transformControl.ActiveAxis());

  // test setting transform mode
  transformControl.SetTransformMode(TransformMode::TM_ROTATION);
  EXPECT_EQ(TransformMode::TM_ROTATION, transformControl.Mode());

  // verify active state
  transformControl.Start();
  EXPECT_TRUE(transformControl.Active());
  transformControl.Stop();
  EXPECT_FALSE(transformControl.Active());

  // test axis conversion
  EXPECT_EQ(math::Vector3d::UnitX,
      transformControl.ToAxis(TransformAxis::TA_TRANSLATION_X));
  EXPECT_EQ(math::Vector3d::UnitY,
      transformControl.ToAxis(TransformAxis::TA_TRANSLATION_Y));
  EXPECT_EQ(math::Vector3d::UnitZ,
      transformControl.ToAxis(TransformAxis::TA_TRANSLATION_Z));
  EXPECT_EQ(math::Vector3d::UnitX,
      transformControl.ToAxis(TransformAxis::TA_ROTATION_X));
  EXPECT_EQ(math::Vector3d::UnitY,
      transformControl.ToAxis(TransformAxis::TA_ROTATION_Y));
  EXPECT_EQ(math::Vector3d::UnitZ,
      transformControl.ToAxis(TransformAxis::TA_ROTATION_Z));
  EXPECT_EQ(math::Vector3d::UnitX,
      transformControl.ToAxis(TransformAxis::TA_SCALE_X));
  EXPECT_EQ(math::Vector3d::UnitY,
      transformControl.ToAxis(TransformAxis::TA_SCALE_Y));
  EXPECT_EQ(math::Vector3d::UnitZ,
      transformControl.ToAxis(TransformAxis::TA_SCALE_Z));

  // Clean up
  engine->DestroyScene(scene);
}

/////////////////////////////////////////////////
TEST_F(TransformControllerTest, WorldSpace)
{
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  CameraPtr camera =  scene->CreateCamera("camera");
  ASSERT_NE(nullptr, camera);

  camera->SetImageWidth(320);
  camera->SetImageHeight(240);

  TransformController transformControl;

  // test setting camera
  transformControl.SetCamera(camera);
  EXPECT_EQ(camera, transformControl.Camera());

  // create visual node for testing
  VisualPtr visual = scene->CreateVisual();
  ASSERT_NE(nullptr, visual);
  transformControl.Attach(visual);
  EXPECT_EQ(visual, transformControl.Node());

  // test translation in world space
  transformControl.SetTransformMode(TransformMode::TM_TRANSLATION);
  transformControl.SetTransformSpace(TransformSpace::TS_WORLD);
  transformControl.SetActiveAxis(math::Vector3d::UnitZ);
  transformControl.Translate(math::Vector3d(0, 0, 2));
  EXPECT_EQ(visual->WorldPosition(), math::Vector3d(0, 0, 2));
  EXPECT_EQ(visual->WorldRotation(), math::Quaterniond::Identity);
  EXPECT_EQ(visual->WorldScale(), math::Vector3d::One);

  // test rotation in world space
  transformControl.SetTransformMode(TransformMode::TM_ROTATION);
  transformControl.SetTransformSpace(TransformSpace::TS_WORLD);
  transformControl.SetActiveAxis(math::Vector3d::UnitX);
  transformControl.Rotate(math::Quaterniond(GZ_PI, 0, 0));
  EXPECT_EQ(visual->WorldPosition(), math::Vector3d(0, 0, 2));
  EXPECT_EQ(visual->WorldRotation(), math::Quaterniond(GZ_PI, 0, 0));
  EXPECT_EQ(visual->WorldScale(), math::Vector3d::One);

  // test scaling in world space
  transformControl.SetTransformMode(TransformMode::TM_SCALE);
  transformControl.SetTransformSpace(TransformSpace::TS_WORLD);
  transformControl.SetActiveAxis(math::Vector3d::UnitY);
  transformControl.Scale(math::Vector3d(1.0, 0.3, 1.0));
  EXPECT_EQ(visual->WorldPosition(), math::Vector3d(0, 0, 2));
  EXPECT_EQ(visual->WorldRotation(), math::Quaterniond(GZ_PI, 0, 0));
  EXPECT_EQ(visual->WorldScale(), math::Vector3d(1.0, 0.3, 1.0));

  // Clean up
  engine->DestroyScene(scene);
}

/////////////////////////////////////////////////
TEST_F(TransformControllerTest, LocalSpace)
{
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  CameraPtr camera =  scene->CreateCamera("camera");
  ASSERT_NE(nullptr, camera);

  camera->SetImageWidth(320);
  camera->SetImageHeight(240);

  TransformController transformControl;

  // test setting camera
  transformControl.SetCamera(camera);
  EXPECT_EQ(camera, transformControl.Camera());

  // create a visual node and intiialize it with a rotation
  // for testing transfoms in local space
  VisualPtr visual = scene->CreateVisual();
  ASSERT_NE(nullptr, visual);
  math::Quaterniond initialRot(GZ_PI * 0.5, 0, 0);
  visual->SetLocalRotation(initialRot);
  EXPECT_EQ(initialRot, visual->WorldRotation());
  transformControl.Attach(visual);
  EXPECT_EQ(visual, transformControl.Node());

  // test translation in local space
  transformControl.SetTransformMode(TransformMode::TM_TRANSLATION);
  transformControl.SetTransformSpace(TransformSpace::TS_LOCAL);
  transformControl.SetActiveAxis(math::Vector3d::UnitZ);
  transformControl.Translate(math::Vector3d(0, 0, 2));
  EXPECT_EQ(visual->WorldPosition(), math::Vector3d(0, -2, 0));
  EXPECT_EQ(visual->WorldRotation(), initialRot);
  EXPECT_EQ(visual->WorldScale(), math::Vector3d::One);

  // test rotation in local space
  transformControl.SetTransformMode(TransformMode::TM_ROTATION);
  transformControl.SetTransformSpace(TransformSpace::TS_LOCAL);
  transformControl.SetActiveAxis(math::Vector3d::UnitX);
  transformControl.Rotate(math::Quaterniond(GZ_PI, 0, 0));
  EXPECT_EQ(visual->WorldPosition(), math::Vector3d(0, -2, 0));
  EXPECT_EQ(visual->WorldRotation(),
      math::Quaterniond(GZ_PI, 0, 0) * initialRot);
  EXPECT_EQ(visual->WorldScale(), math::Vector3d::One);

  // test scaling in local space
  transformControl.SetTransformMode(TransformMode::TM_SCALE);
  transformControl.SetTransformSpace(TransformSpace::TS_LOCAL);
  transformControl.SetActiveAxis(math::Vector3d::UnitY);
  transformControl.Scale(math::Vector3d(1.0, 0.3, 1.0));
  EXPECT_EQ(visual->WorldPosition(), math::Vector3d(0, -2, 0));
  EXPECT_EQ(visual->WorldRotation(),
      math::Quaterniond(GZ_PI, 0, 0) * initialRot);
  EXPECT_EQ(visual->WorldScale(), math::Vector3d(1.0, 0.3, 1.0));

  // Clean up
  engine->DestroyScene(scene);
}

/////////////////////////////////////////////////
TEST_F(TransformControllerTest, Control2d)
{
  ScenePtr scene = engine->CreateScene("scene");
  ASSERT_NE(nullptr, scene);

  CameraPtr camera =  scene->CreateCamera("camera");
  ASSERT_NE(nullptr, camera);
  camera->SetWorldPosition(-5, 0, 0);
  EXPECT_EQ(math::Vector3d(-5, 0, 0), camera->WorldPosition());

  camera->SetImageWidth(320);
  camera->SetImageHeight(240);

  TransformController transformControl;

  // test setting camera
  transformControl.SetCamera(camera);
  EXPECT_EQ(camera, transformControl.Camera());

  // create a dummy node visual node and attach to the controller
  VisualPtr visual = scene->CreateVisual();
  ASSERT_NE(nullptr, visual);
  transformControl.Attach(visual);
  EXPECT_EQ(visual, transformControl.Node());

  // test translation from 2d
  transformControl.SetTransformMode(TransformMode::TM_TRANSLATION);
  transformControl.SetTransformSpace(TransformSpace::TS_LOCAL);
  transformControl.SetActiveAxis(math::Vector3d::UnitZ);
  transformControl.Start();
  math::Vector2d start(0.5, 0.5);
  math::Vector2d end(0.5, 0.8);
  math::Vector3d translation =
      transformControl.TranslationFrom2d(math::Vector3d::UnitZ, start, end);
  transformControl.Stop();
  EXPECT_DOUBLE_EQ(translation.X(), 0);
  EXPECT_DOUBLE_EQ(translation.Y(), 0);
  EXPECT_GT(translation.Z(), 0);

  // test rotation from 2d
  transformControl.SetTransformMode(TransformMode::TM_ROTATION);
  transformControl.SetTransformSpace(TransformSpace::TS_LOCAL);
  transformControl.SetActiveAxis(math::Vector3d::UnitX);
  transformControl.Start();
  start = math::Vector2d(0.5, 0.5);
  end = math::Vector2d(0.5, -0.8);
  math::Quaterniond rotation =
      transformControl.RotationFrom2d(math::Vector3d::UnitX, start, end);
  transformControl.Stop();
  math::Vector3d euler = rotation.Euler();
  EXPECT_GT(euler.X(), 0);
  EXPECT_DOUBLE_EQ(euler.Y(), 0);
  EXPECT_DOUBLE_EQ(euler.Z(), 0);

  // test scaling from 2d
  transformControl.SetTransformMode(TransformMode::TM_SCALE);
  transformControl.SetTransformSpace(TransformSpace::TS_LOCAL);
  transformControl.SetActiveAxis(math::Vector3d::UnitY);
  transformControl.Start();
  math::Vector3d scale =
      transformControl.ScaleFrom2d(math::Vector3d::UnitY, start, end);
  transformControl.Stop();
  EXPECT_DOUBLE_EQ(scale.X(), 1);
  EXPECT_GT(scale.Y(), 0);
  EXPECT_DOUBLE_EQ(scale.Z(), 1);

  // Clean up
  engine->DestroyScene(scene);
}
