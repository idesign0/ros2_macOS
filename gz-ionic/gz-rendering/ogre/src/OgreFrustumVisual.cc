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

#include <array>
#include <vector>

#include <gz/common/Console.hh>
#include "gz/rendering/ogre/OgreDynamicLines.hh"
#include "gz/rendering/ogre/OgreFrustumVisual.hh"
#include "gz/rendering/ogre/OgreScene.hh"
#include "gz/rendering/ogre/OgreMarker.hh"
#include "gz/rendering/ogre/OgreGeometry.hh"


class gz::rendering::OgreFrustumVisualPrivate
{
  /// \brief Frustum Ray DynamicLines Object to display
  public: std::vector<std::shared_ptr<OgreDynamicLines>> rayLines;

  /// \brief The visibility of the visual
  public: bool visible = true;
};

using namespace gz;
using namespace rendering;

//////////////////////////////////////////////////
OgreFrustumVisual::OgreFrustumVisual()
  : dataPtr(new OgreFrustumVisualPrivate)
{
}

//////////////////////////////////////////////////
OgreFrustumVisual::~OgreFrustumVisual()
{
  // no ops
}

//////////////////////////////////////////////////
void OgreFrustumVisual::PreRender()
{
  // no ops
}

//////////////////////////////////////////////////
void OgreFrustumVisual::Destroy()
{
  BaseFrustumVisual::Destroy();
}

//////////////////////////////////////////////////
void OgreFrustumVisual::Init()
{
  BaseFrustumVisual::Init();
  this->Create();
}

//////////////////////////////////////////////////
void OgreFrustumVisual::Create()
{
  // no ops
}

//////////////////////////////////////////////////
void OgreFrustumVisual::ClearVisualData()
{
  this->dataPtr->rayLines.clear();
}

//////////////////////////////////////////////////
void OgreFrustumVisual::Update()
{
  // no ops
}

//////////////////////////////////////////////////
void OgreFrustumVisual::SetVisible(bool _visible)
{
  this->dataPtr->visible = _visible;
  this->ogreNode->setVisible(this->dataPtr->visible);
}
