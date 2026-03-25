/*
 * Copyright (C) 2025 Open Source Robotics Foundation
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

#ifdef __APPLE__
  #define GL_SILENCE_DEPRECATION
  #include <OpenGL/gl.h>
  #include <OpenGL/glext.h>
#else
#ifndef _WIN32
  #include <GL/gl.h>
#endif
#endif

#include <array>
#include <cmath>
#include <memory>
#include <string>
#include <vector>

#include <gz/common/Console.hh>

#include "gz/rendering/ogre2/Ogre2Conversions.hh"
#include "gz/rendering/ogre2/Ogre2DynamicRenderable.hh"
#include "gz/rendering/ogre2/Ogre2FrustumVisual.hh"
#include "gz/rendering/ogre2/Ogre2RenderEngine.hh"
#include "gz/rendering/ogre2/Ogre2Scene.hh"
#include "gz/rendering/ogre2/Ogre2Marker.hh"
#include "gz/rendering/ogre2/Ogre2Geometry.hh"

#ifdef _MSC_VER
  #pragma warning(push, 0)
#endif
#include <OgreItem.h>
#include <OgreMaterialManager.h>
#include <OgreRoot.h>
#include <OgreSceneNode.h>
#include <OgreTechnique.h>
#ifdef _MSC_VER
  #pragma warning(pop)
#endif

class gz::rendering::Ogre2FrustumVisualPrivate
{
  /// \brief Frustum Ray DynamicLines Object to display
  public: std::vector<std::shared_ptr<Ogre2DynamicRenderable>> rayLines;

  /// \brief The visibility of the visual
  public: bool visible = true;

  /// \brief Each corner of the frustum.
  public: std::array<gz::math::Vector3d, 8> points;

  /// \brief each edge of the frustum.
  public: std::array<std::pair<gz::math::Vector3d,
    gz::math::Vector3d>, 12> edges;
};

using namespace gz;
using namespace rendering;

//////////////////////////////////////////////////
Ogre2FrustumVisual::Ogre2FrustumVisual()
  : dataPtr(new Ogre2FrustumVisualPrivate)
{
}

//////////////////////////////////////////////////
Ogre2FrustumVisual::~Ogre2FrustumVisual()
{
  // no ops
}

//////////////////////////////////////////////////
void Ogre2FrustumVisual::PreRender()
{
  // no ops
}

//////////////////////////////////////////////////
void Ogre2FrustumVisual::Destroy()
{
  BaseFrustumVisual::Destroy();
}

//////////////////////////////////////////////////
void Ogre2FrustumVisual::Init()
{
  BaseFrustumVisual::Init();
  this->Create();
}

//////////////////////////////////////////////////
void Ogre2FrustumVisual::Create()
{
  // enable GL_PROGRAM_POINT_SIZE so we can set gl_PointSize in vertex shader
  auto engine = Ogre2RenderEngine::Instance();
  std::string renderSystemName =
      engine->OgreRoot()->getRenderSystem()->getFriendlyName();
  if (renderSystemName.find("OpenGL") != std::string::npos)
  {
#ifdef __APPLE__
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
#else
#ifndef _WIN32
    glEnable(GL_PROGRAM_POINT_SIZE);
#endif
#endif
  }
}

//////////////////////////////////////////////////
void Ogre2FrustumVisual::ClearVisualData()
{
  this->dataPtr->rayLines.clear();
}

//////////////////////////////////////////////////
void Ogre2FrustumVisual::Update()
{
  std::shared_ptr<Ogre2DynamicRenderable> renderable =
                  std::shared_ptr<Ogre2DynamicRenderable>(
                              new Ogre2DynamicRenderable(this->Scene()));
  this->ogreNode->attachObject(renderable->OgreObject());

  #if (!(OGRE_VERSION <= ((1 << 16) | (10 << 8) | 7)))
  // the Materials are assigned here to avoid repetitive search for materials
  Ogre::MaterialPtr rayLineMat =
                  Ogre::MaterialManager::getSingleton().getByName(
                                                    "Frustum/BlueRay");
  #endif

  #if (OGRE_VERSION <= ((1 << 16) | (10 << 8) | 7))
    MaterialPtr mat = this->Scene()->Material("Frustum/BlueRay");
  #else
    MaterialPtr mat = this->Scene()->Material("Frustum/BlueRay");
  #endif

  renderable->SetMaterial(mat, false);
  renderable->SetOperationType(MT_LINE_LIST);
  this->dataPtr->rayLines.push_back(renderable);

  // Tangent of half the field of view.
  double tanFOV2 = std::tan(this->hfov() * 0.5);

  // Width of near plane
  double nearWidth = 2.0 * tanFOV2 * this->nearClip;

  // Height of near plane
  double nearHeight = nearWidth / this->aspectRatio;

  // Width of far plane
  double farWidth = 2.0 * tanFOV2 * this->farClip;

  // Height of far plane
  double farHeight = farWidth / this->aspectRatio;

  // Up, right, and forward unit vectors.
  gz::math::Vector3d forward =
    this->pose.Rot().RotateVector(gz::math::Vector3d::UnitX);
  gz::math::Vector3d up =
    this->pose.Rot().RotateVector(gz::math::Vector3d::UnitZ);
  gz::math::Vector3d right =
    this->pose.Rot().RotateVector(-gz::math::Vector3d::UnitY);

  // Near plane center
  gz::math::Vector3d nearCenter = this->pose.Pos() + forward * this->nearClip;

  // Far plane center
  gz::math::Vector3d farCenter = this->pose.Pos() + forward * this->farClip;

  // These four variables are here for convenience.
  gz::math::Vector3d upNearHeight2 = up * (nearHeight * 0.5);
  gz::math::Vector3d rightNearWidth2 = right * (nearWidth * 0.5);
  gz::math::Vector3d upFarHeight2 = up * (farHeight * 0.5);
  gz::math::Vector3d rightFarWidth2 = right * (farWidth * 0.5);

  // Compute the vertices of the near plane
  gz::math::Vector3d nearTopLeft =
    nearCenter + upNearHeight2 - rightNearWidth2;
  gz::math::Vector3d nearTopRight =
    nearCenter + upNearHeight2 + rightNearWidth2;
  gz::math::Vector3d nearBottomLeft =
    nearCenter - upNearHeight2 - rightNearWidth2;
  gz::math::Vector3d nearBottomRight =
    nearCenter - upNearHeight2 + rightNearWidth2;

  // Compute the vertices of the far plane
  gz::math::Vector3d farTopLeft = farCenter + upFarHeight2 - rightFarWidth2;
  gz::math::Vector3d farTopRight = farCenter + upFarHeight2 + rightFarWidth2;
  gz::math::Vector3d farBottomLeft = farCenter - upFarHeight2 - rightFarWidth2;
  gz::math::Vector3d farBottomRight = farCenter - upFarHeight2 + rightFarWidth2;

  // Save these vertices
  this->dataPtr->points[0] = nearTopLeft;
  this->dataPtr->points[1] = nearTopRight;
  this->dataPtr->points[2] = nearBottomLeft;
  this->dataPtr->points[3] = nearBottomRight;
  this->dataPtr->points[4] = farTopLeft;
  this->dataPtr->points[5] = farTopRight;
  this->dataPtr->points[6] = farBottomLeft;
  this->dataPtr->points[7] = farBottomRight;

  // Save the edges
  this->dataPtr->edges[0] = {nearTopLeft, nearTopRight};
  this->dataPtr->edges[1] = {nearTopLeft, nearBottomLeft};
  this->dataPtr->edges[2] = {nearTopLeft, farTopLeft};
  this->dataPtr->edges[3] = {nearTopRight, nearBottomRight};
  this->dataPtr->edges[4] = {nearTopRight, farTopRight};
  this->dataPtr->edges[5] = {nearBottomLeft, nearBottomRight};
  this->dataPtr->edges[6] = {nearBottomLeft, farBottomLeft};
  this->dataPtr->edges[7] = {farTopLeft, farTopRight};
  this->dataPtr->edges[8] = {farTopLeft, farBottomLeft};
  this->dataPtr->edges[9] = {farTopRight, farBottomRight};
  this->dataPtr->edges[10] = {farBottomLeft, farBottomRight};
  this->dataPtr->edges[11] = {farBottomRight, nearBottomRight};

  gz::math::Vector3d leftCenter =
    (farTopLeft + nearTopLeft + farBottomLeft + nearBottomLeft) / 4.0;

  gz::math::Vector3d rightCenter =
    (farTopRight + nearTopRight + farBottomRight + nearBottomRight) / 4.0;

  gz::math::Vector3d topCenter =
    (farTopRight + nearTopRight + farTopLeft + nearTopLeft) / 4.0;

  gz::math::Vector3d bottomCenter =
    (farBottomRight + nearBottomRight + farBottomLeft + nearBottomLeft) / 4.0;

  // For creating the frustum visuals
  renderable->AddPoint(math::Vector3d(this->nearClip, nearWidth, nearHeight));
  renderable->AddPoint(math::Vector3d(this->nearClip, nearWidth, -nearHeight));
  renderable->AddPoint(math::Vector3d(this->nearClip, nearWidth, -nearHeight));
  renderable->AddPoint(math::Vector3d(this->nearClip, -nearWidth, -nearHeight));
  renderable->AddPoint(math::Vector3d(this->nearClip, -nearWidth, -nearHeight));
  renderable->AddPoint(math::Vector3d(this->nearClip, -nearWidth, nearHeight));
  renderable->AddPoint(math::Vector3d(this->nearClip, -nearWidth, nearHeight));
  renderable->AddPoint(math::Vector3d(this->nearClip, nearWidth, nearHeight));
  renderable->AddPoint(math::Vector3d(this->farClip, farWidth, farHeight));
  renderable->AddPoint(math::Vector3d(this->farClip, farWidth, -farHeight));
  renderable->AddPoint(math::Vector3d(this->farClip, farWidth, -farHeight));
  renderable->AddPoint(math::Vector3d(this->farClip, -farWidth, -farHeight));
  renderable->AddPoint(math::Vector3d(this->farClip, -farWidth, -farHeight));
  renderable->AddPoint(math::Vector3d(this->farClip, -farWidth, farHeight));
  renderable->AddPoint(math::Vector3d(this->farClip, -farWidth, farHeight));
  renderable->AddPoint(math::Vector3d(this->farClip, farWidth, farHeight));
  renderable->AddPoint(math::Vector3d(this->nearClip, nearWidth, nearHeight));
  renderable->AddPoint(math::Vector3d(this->farClip, farWidth, farHeight));
  renderable->AddPoint(math::Vector3d(this->nearClip, -nearWidth, nearHeight));
  renderable->AddPoint(math::Vector3d(this->farClip, -farWidth, farHeight));
  renderable->AddPoint(math::Vector3d(this->nearClip, -nearWidth, -nearHeight));
  renderable->AddPoint(math::Vector3d(this->farClip, -farWidth, -farHeight));
  renderable->AddPoint(math::Vector3d(this->nearClip, nearWidth, -nearHeight));
  renderable->AddPoint(math::Vector3d(this->farClip, farWidth, -farHeight));

  // Compute plane offsets
  // Set the planes, where the first value is the plane normal and the
  // second the plane offset
  gz::math::Vector3d norm = gz::math::Vector3d::Normal(
    nearTopLeft, nearTopRight, nearBottomLeft);
  this->planes[FrustumVisualPlane::FRUSTUM_PLANE_NEAR].Set(
    norm, nearCenter.Dot(norm));

  norm = gz::math::Vector3d::Normal(
    farTopRight, farTopLeft, farBottomLeft);
  this->planes[FrustumVisualPlane::FRUSTUM_PLANE_FAR].Set(
    norm, farCenter.Dot(norm));

  norm = gz::math::Vector3d::Normal(
    farTopLeft, nearTopLeft, nearBottomLeft);
  this->planes[FrustumVisualPlane::FRUSTUM_PLANE_LEFT].Set(
    norm, leftCenter.Dot(norm));

  norm = gz::math::Vector3d::Normal(
    nearTopRight, farTopRight, farBottomRight);
  this->planes[FrustumVisualPlane::FRUSTUM_PLANE_RIGHT].Set(
    norm, rightCenter.Dot(norm));

  norm = gz::math::Vector3d::Normal(
    nearTopLeft, farTopLeft, nearTopRight);
  this->planes[FrustumVisualPlane::FRUSTUM_PLANE_TOP].Set(
    norm, topCenter.Dot(norm));

  norm = gz::math::Vector3d::Normal(
    nearBottomLeft, nearBottomRight, farBottomRight);
  this->planes[FrustumVisualPlane::FRUSTUM_PLANE_BOTTOM].Set(
    norm, bottomCenter.Dot(norm));

  renderable->Update();
  this->SetVisible(this->dataPtr->visible);
}

//////////////////////////////////////////////////
void Ogre2FrustumVisual::SetVisible(bool _visible)
{
  this->dataPtr->visible = _visible;
  this->ogreNode->setVisible(this->dataPtr->visible);
}
