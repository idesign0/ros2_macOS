/* * Copyright (C) 2018 Open Source Robotics Foundation
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

#include "gz/rendering/Text.hh"
#include "gz/rendering/Scene.hh"

using namespace gz;
using namespace rendering;

class TextTest : public CommonRenderingTest 
{
};

/////////////////////////////////////////////////
TEST_F(TextTest, Text)
{
  CHECK_SUPPORTED_ENGINE("ogre");

  ScenePtr scene = engine->CreateScene("scene");

  TextPtr text = scene->CreateText();
  ASSERT_NE(nullptr, text);

  // check default values
  EXPECT_NE(std::string(), text->FontName());
  EXPECT_EQ(std::string(), text->TextString());
  EXPECT_FLOAT_EQ(0.0, text->SpaceWidth());
  EXPECT_FLOAT_EQ(1.0, text->CharHeight());
  EXPECT_FLOAT_EQ(0.0, text->Baseline());
  EXPECT_EQ(TextHorizontalAlign::LEFT, text->HorizontalAlignment());
  EXPECT_EQ(TextVerticalAlign::BOTTOM, text->VerticalAlignment());
  EXPECT_EQ(math::Color::White, text->Color());
  EXPECT_FALSE(text->ShowOnTop());
  EXPECT_NE(math::AxisAlignedBox(), text->AABB());

  // check setting text properties
  text->SetFontName("Liberation Sans");
  EXPECT_EQ("Liberation Sans", text->FontName());

  text->SetTextString("abc def");
  EXPECT_EQ("abc def", text->TextString());

  text->SetCharHeight(1.8f);
  EXPECT_FLOAT_EQ(1.8f, text->CharHeight());

  text->SetSpaceWidth(1.5f);
  EXPECT_FLOAT_EQ(1.5f, text->SpaceWidth());

  text->SetBaseline(0.5f);
  EXPECT_FLOAT_EQ(0.5f, text->Baseline());

  text->SetTextAlignment(TextHorizontalAlign::CENTER, TextVerticalAlign::TOP);
  EXPECT_EQ(TextHorizontalAlign::CENTER, text->HorizontalAlignment());
  EXPECT_EQ(TextVerticalAlign::TOP, text->VerticalAlignment());

  text->SetShowOnTop(true);
  EXPECT_TRUE(text->ShowOnTop());

  text->SetColor(math::Color(1.0f, 0.2f, 0.3f, 1.0f));
  EXPECT_EQ(math::Color(1.0f, 0.2f, 0.3f, 1.0f), text->Color());

  // create material
  MaterialPtr mat = scene->CreateMaterial();
  mat->SetAmbient(0.6f, 0.7f, 0.8f);
  mat->SetDiffuse(0.3f, 0.8f, 0.2f);
  mat->SetSpecular(0.4f, 0.9f, 1.0f);

  text->SetMaterial(mat);
  MaterialPtr textMat = text->Material();
  ASSERT_NE(nullptr, textMat);
  EXPECT_EQ(math::Color(0.6f, 0.7f, 0.8f), textMat->Ambient());
  EXPECT_EQ(math::Color(0.3f, 0.8f, 0.2f), textMat->Diffuse());
  EXPECT_EQ(math::Color(0.4f, 0.9f, 1.0f), textMat->Specular());

  // color is affected by material but currently only by the diffuse component
  EXPECT_EQ(math::Color(0.3f, 0.8f, 0.2f, 1.0f), text->Color());

  // Clean up
  engine->DestroyScene(scene);
}

class FontTest : public TextTest, public testing::WithParamInterface<std::string> {};

TEST_P(FontTest, SupportedFont){
  CHECK_SUPPORTED_ENGINE("ogre");

  ScenePtr scene = engine->CreateScene("scene");

  TextPtr text = scene->CreateText();
  ASSERT_NE(nullptr, text);

  const auto& font = GetParam();
  // check setting text properties
  text->SetFontName(font);
  EXPECT_EQ(font, text->FontName());

  text->SetTextString("abc def");
  EXPECT_EQ("abc def", text->TextString());

  // Font searching happens during PreRender.
  text->PreRender();

  // Clean up
  engine->DestroyScene(scene);
}

INSTANTIATE_TEST_SUITE_P(FontTestInstantiation,
                         FontTest,
                         testing::Values("Console", "Liberation Sans", "Roboto"));
